#!/usr/bin/env python3
"""Compare TCP camera capture with the Windows shared-memory transport."""

import argparse
import json
import mmap
import statistics
import sys
import time
from datetime import datetime, timezone
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT / "client" / "python"))

import unrealcv


MODALITIES = {
    "lit": ("png", "lit_shared"),
    "depth": ("npy", "depth_shared"),
    "normal": ("png", "normal_shared"),
    "object_mask": ("png", "object_mask_shared"),
}


def percentile(values, fraction):
    ordered = sorted(values)
    index = min(len(ordered) - 1, round((len(ordered) - 1) * fraction))
    return ordered[index]


def summarize(latencies_ms, payload_bytes):
    average = statistics.mean(latencies_ms)
    return {
        "iterations": len(latencies_ms),
        "mean_ms": average,
        "median_ms": statistics.median(latencies_ms),
        "p95_ms": percentile(latencies_ms, 0.95),
        "min_ms": min(latencies_ms),
        "max_ms": max(latencies_ms),
        "fps": 1000.0 / average,
        "payload_bytes": payload_bytes,
    }


def request_tcp(client, command):
    start = time.perf_counter()
    response = client.request(command)
    elapsed_ms = (time.perf_counter() - start) * 1000.0
    if response is None or (isinstance(response, str) and response.startswith("error")):
        raise RuntimeError(f"TCP capture failed: {response}")
    return elapsed_ms, len(response)


def request_shared(client, command):
    start = time.perf_counter()
    response = client.request(command)
    metadata = json.loads(response)
    with mmap.mmap(
        -1,
        metadata["num_bytes"],
        tagname=metadata["name"],
        access=mmap.ACCESS_READ,
    ) as mapping:
        mapping[:]
    elapsed_ms = (time.perf_counter() - start) * 1000.0
    return elapsed_ms, metadata["num_bytes"], metadata


def benchmark(client, camera_id, modality, iterations, warmup):
    tcp_format, shared_route = MODALITIES[modality]
    tcp_command = f"vget /camera/{camera_id}/{modality} {tcp_format}"
    shared_command = f"vget /camera/{camera_id}/{shared_route}"

    for _ in range(warmup):
        request_tcp(client, tcp_command)
        request_shared(client, shared_command)

    tcp_times = []
    shared_times = []
    tcp_bytes = 0
    shared_bytes = 0
    metadata = None
    for _ in range(iterations):
        elapsed, tcp_bytes = request_tcp(client, tcp_command)
        tcp_times.append(elapsed)
        elapsed, shared_bytes, metadata = request_shared(client, shared_command)
        shared_times.append(elapsed)

    tcp = summarize(tcp_times, tcp_bytes)
    shared = summarize(shared_times, shared_bytes)
    return {
        "tcp_command": tcp_command,
        "shared_command": shared_command,
        "tcp": tcp,
        "shared_memory": shared,
        "speedup_mean": tcp["mean_ms"] / shared["mean_ms"],
        "latency_reduction_percent": (1.0 - shared["mean_ms"] / tcp["mean_ms"]) * 100.0,
        "shared_metadata": metadata,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--camera-id", type=int, default=0)
    parser.add_argument("--iterations", type=int, default=50)
    parser.add_argument("--warmup", type=int, default=5)
    parser.add_argument("--modalities", nargs="+", choices=MODALITIES, default=list(MODALITIES))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()

    if sys.platform != "win32":
        parser.error("shared-memory camera capture is currently Windows-only")

    client = unrealcv.Client((args.host, args.port))
    if not client.connect(timeout=10):
        raise RuntimeError(f"Could not connect to UnrealCV at {args.host}:{args.port}")

    try:
        size = client.request(f"vget /camera/{args.camera_id}/size")
        report = {
            "timestamp_utc": datetime.now(timezone.utc).isoformat(),
            "server": f"{args.host}:{args.port}",
            "camera_id": args.camera_id,
            "camera_size": size,
            "iterations": args.iterations,
            "warmup": args.warmup,
            "measurement": (
                "End-to-end client acquisition. TCP returns encoded PNG/NPY bytes; shared memory returns raw "
                "BGRA uint8 or float32 data and includes copying the full mapping into Python bytes."
            ),
            "modalities": {},
        }
        for modality in args.modalities:
            result = benchmark(client, args.camera_id, modality, args.iterations, args.warmup)
            report["modalities"][modality] = result
            print(
                f"{modality:12} TCP {result['tcp']['mean_ms']:8.2f} ms | "
                f"shared {result['shared_memory']['mean_ms']:8.2f} ms | "
                f"speedup {result['speedup_mean']:.2f}x"
            )
    finally:
        client.disconnect()

    output = args.output or REPO_ROOT / ".artifacts" / "shared_memory_ab.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Report: {output}")


if __name__ == "__main__":
    main()

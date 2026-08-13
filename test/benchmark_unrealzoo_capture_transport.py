#!/usr/bin/env python3
"""Benchmark UnrealZoo TCP and Windows shared-memory camera acquisition."""

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


STANDARD_RESOLUTIONS = [
    (640, 480, "480p"),
    (1280, 720, "720p"),
    (1920, 1080, "1080p"),
    (2560, 1440, "2K"),
    (3840, 2160, "4K"),
    (7680, 4320, "8K"),
]


def percentile(values, fraction):
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1, round((len(ordered) - 1) * fraction))]


def summary(values, payload_bytes):
    average = statistics.mean(values)
    return {
        "iterations": len(values),
        "mean_ms": average,
        "median_ms": statistics.median(values),
        "p95_ms": percentile(values, 0.95),
        "min_ms": min(values),
        "max_ms": max(values),
        "fps": 1000.0 / average,
        "payload_bytes": payload_bytes,
    }


def read_shared(response):
    metadata = json.loads(response)
    with mmap.mmap(-1, metadata["num_bytes"], tagname=metadata["name"], access=mmap.ACCESS_READ) as mapping:
        mapping[:]
    return metadata


def time_request(client, command, shared):
    started = time.perf_counter()
    response = client.request(command)
    elapsed_ms = (time.perf_counter() - started) * 1000.0
    if response is None or (isinstance(response, str) and response.startswith("error")):
        raise RuntimeError(f"Capture failed for {command}: {response}")
    if shared:
        return elapsed_ms, read_shared(response)["num_bytes"]
    return elapsed_ms, len(response)


def benchmark_pair(client, tcp_command, shared_command, iterations, warmup):
    for _ in range(warmup):
        time_request(client, tcp_command, False)
        time_request(client, shared_command, True)

    tcp_times = []
    shared_times = []
    tcp_bytes = shared_bytes = 0
    for _ in range(iterations):
        elapsed, tcp_bytes = time_request(client, tcp_command, False)
        tcp_times.append(elapsed)
        elapsed, shared_bytes = time_request(client, shared_command, True)
        shared_times.append(elapsed)

    tcp = summary(tcp_times, tcp_bytes)
    shared = summary(shared_times, shared_bytes)
    return {
        "tcp_command": tcp_command,
        "shared_command": shared_command,
        "tcp": tcp,
        "shared_memory": shared,
        "speedup_mean": tcp["mean_ms"] / shared["mean_ms"],
        "latency_reduction_percent": (1.0 - shared["mean_ms"] / tcp["mean_ms"]) * 100.0,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--camera-id", type=int, default=0)
    parser.add_argument("--iterations", type=int, default=10)
    parser.add_argument("--warmup", type=int, default=3)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if sys.platform != "win32":
        parser.error("Windows named shared memory is required")

    client = unrealcv.Client((args.host, args.port))
    if not client.connect(timeout=10):
        raise RuntimeError(f"Could not connect to UnrealCV at {args.host}:{args.port}")

    report = {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "server": f"{args.host}:{args.port}",
        "camera_id": args.camera_id,
        "iterations": args.iterations,
        "warmup": args.warmup,
        "measurement": "End-to-end Python acquisition. TCP fully receives raw BMP bytes; shared memory parses JSON, opens the named mapping, and copies every mapped byte.",
        "standard_camera": {},
        "panorama": {},
    }
    try:
        for width, height, label in STANDARD_RESOLUTIONS:
            response = client.request(f"vset /camera/{args.camera_id}/size {width} {height}")
            if response is None or str(response).startswith("error"):
                raise RuntimeError(f"Could not set camera size {width}x{height}: {response}")
            result = benchmark_pair(
                client,
                f"vget /camera/{args.camera_id}/lit bmp",
                f"vget /camera/{args.camera_id}/lit_shared",
                args.iterations,
                args.warmup,
            )
            report["standard_camera"][label] = {"width": width, "height": height, **result}
            print(f"standard {label:5} TCP {result['tcp']['mean_ms']:.2f} ms | shared {result['shared_memory']['mean_ms']:.2f} ms | {result['speedup_mean']:.2f}x")

        for width, height, label in STANDARD_RESOLUTIONS:
            result = benchmark_pair(
                client,
                f"vget /camera/{args.camera_id}/panoramic bmp {width} {height}",
                f"vget /camera/{args.camera_id}/panoramic_shared {width} {height}",
                args.iterations,
                args.warmup,
            )
            report["panorama"][label] = {"width": width, "height": height, **result}
            print(f"panorama {label:5} TCP {result['tcp']['mean_ms']:.2f} ms | shared {result['shared_memory']['mean_ms']:.2f} ms | {result['speedup_mean']:.2f}x")
    finally:
        client.disconnect()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Report: {args.output}")


if __name__ == "__main__":
    main()

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


def set_camera_pose(client, camera_id, location, rotation):
    for command in (
        f"vset /camera/{camera_id}/location {' '.join(map(str, location))}",
        f"vset /camera/{camera_id}/rotation {' '.join(map(str, rotation))}",
    ):
        response = client.request(command)
        if response is None or str(response).startswith("error"):
            raise RuntimeError(f"Could not set camera pose with {command}: {response}")


def time_request(client, command, shared):
    started = time.perf_counter()
    response = client.request(command)
    if response is None or (isinstance(response, str) and response.startswith("error")):
        raise RuntimeError(f"Capture failed for {command}: {response}")
    if shared:
        payload_bytes = read_shared(response)["num_bytes"]
    else:
        payload_bytes = len(response)
    elapsed_ms = (time.perf_counter() - started) * 1000.0
    return elapsed_ms, payload_bytes


def benchmark_pair(client, tcp_command, shared_command, iterations, warmup, rounds):
    round_results = []
    all_tcp_times = []
    all_shared_times = []
    tcp_bytes = shared_bytes = 0

    for round_index in range(rounds):
        for warmup_index in range(warmup):
            if (round_index + warmup_index) % 2 == 0:
                time_request(client, tcp_command, False)
                time_request(client, shared_command, True)
            else:
                time_request(client, shared_command, True)
                time_request(client, tcp_command, False)

        tcp_times = []
        shared_times = []
        for iteration in range(iterations):
            if (round_index + iteration) % 2 == 0:
                elapsed, tcp_bytes = time_request(client, tcp_command, False)
                tcp_times.append(elapsed)
                elapsed, shared_bytes = time_request(client, shared_command, True)
                shared_times.append(elapsed)
            else:
                elapsed, shared_bytes = time_request(client, shared_command, True)
                shared_times.append(elapsed)
                elapsed, tcp_bytes = time_request(client, tcp_command, False)
                tcp_times.append(elapsed)

        all_tcp_times.extend(tcp_times)
        all_shared_times.extend(shared_times)
        round_results.append({
            "round": round_index + 1,
            "tcp": summary(tcp_times, tcp_bytes),
            "shared_memory": summary(shared_times, shared_bytes),
        })

    tcp = summary(all_tcp_times, tcp_bytes)
    shared = summary(all_shared_times, shared_bytes)
    return {
        "tcp_command": tcp_command,
        "shared_command": shared_command,
        "tcp": tcp,
        "shared_memory": shared,
        "speedup_mean": tcp["mean_ms"] / shared["mean_ms"],
        "latency_reduction_percent": (1.0 - shared["mean_ms"] / tcp["mean_ms"]) * 100.0,
        "rounds": round_results,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9001)
    parser.add_argument("--camera-id", type=int, default=0)
    parser.add_argument("--expected-map", default="Tokyo")
    parser.add_argument("--location", type=float, nargs=3, default=(-5160.632, -1029.995, 138.643))
    parser.add_argument("--rotation", type=float, nargs=3, default=(0.0, -1.002, 0.0))
    parser.add_argument("--startup-settle-seconds", type=float, default=30.0)
    parser.add_argument("--iterations", type=int, default=20)
    parser.add_argument("--warmup", type=int, default=10)
    parser.add_argument("--rounds", type=int, default=3)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if sys.platform != "win32":
        parser.error("Windows named shared memory is required")

    client = unrealcv.Client((args.host, args.port))
    if not client.connect(timeout=10):
        raise RuntimeError(f"Could not connect to UnrealCV at {args.host}:{args.port}")

    actual_map = str(client.request("vget /level/name")).strip()
    if actual_map != args.expected_map:
        client.disconnect()
        raise RuntimeError(f"Expected map {args.expected_map!r}, got {actual_map!r}")
    time.sleep(args.startup_settle_seconds)

    report = {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "server": f"{args.host}:{args.port}",
        "camera_id": args.camera_id,
        "map": actual_map,
        "fixed_camera_location": args.location,
        "fixed_camera_rotation": args.rotation,
        "startup_settle_seconds": args.startup_settle_seconds,
        "iterations": args.iterations,
        "warmup": args.warmup,
        "rounds": args.rounds,
        "measured_samples_per_transport": args.iterations * args.rounds,
        "warmup_samples_per_transport": args.warmup * args.rounds,
        "measurement": "End-to-end Python acquisition. TCP fully receives raw BMP bytes; shared memory parses JSON, opens the named mapping, and copies every mapped byte.",
        "standard_camera": {},
        "mqrc": {},
    }
    try:
        for width, height, label in STANDARD_RESOLUTIONS:
            set_camera_pose(client, args.camera_id, args.location, args.rotation)
            response = client.request(f"vset /camera/{args.camera_id}/size {width} {height}")
            if response is None or str(response).startswith("error"):
                raise RuntimeError(f"Could not set camera size {width}x{height}: {response}")
            result = benchmark_pair(
                client,
                f"vget /camera/{args.camera_id}/lit bmp",
                f"vget /camera/{args.camera_id}/lit_shared",
                args.iterations,
                args.warmup,
                args.rounds,
            )
            report["standard_camera"][label] = {"width": width, "height": height, **result}
            print(f"standard {label:5} TCP {result['tcp']['mean_ms']:.2f} ms | shared {result['shared_memory']['mean_ms']:.2f} ms | {result['speedup_mean']:.2f}x")

        for width, height, label in STANDARD_RESOLUTIONS:
            set_camera_pose(client, args.camera_id, args.location, args.rotation)
            response = client.request(f"vset /camera/{args.camera_id}/size {width} {height}")
            if response is None or str(response).startswith("error"):
                raise RuntimeError(f"Could not set camera size {width}x{height}: {response}")
            result = benchmark_pair(
                client,
                f"vget /camera/{args.camera_id}/mqrc/lit bmp",
                f"vget /camera/{args.camera_id}/mqrc/lit_shared",
                args.iterations,
                args.warmup,
                args.rounds,
            )
            report["mqrc"][label] = {"width": width, "height": height, **result}
            print(f"mqrc {label:5} TCP {result['tcp']['mean_ms']:.2f} ms | shared {result['shared_memory']['mean_ms']:.2f} ms | {result['speedup_mean']:.2f}x")
    finally:
        client.disconnect()

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"Report: {args.output}")


if __name__ == "__main__":
    main()

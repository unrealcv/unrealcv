"""Helpers for building extended UnrealCV Groom wind payloads."""

import json


def build_keyframe_groom_wind_json(direction_keys, strength_keys, indent=2):
    """Build a deterministic JSON payload for keyframed Groom wind."""
    payload = {
        "mode": "keyframe",
        "direction_keys": [
            {"time": float(time_seconds), "value": [float(v) for v in direction]}
            for time_seconds, direction in direction_keys
        ],
        "strength_keys": [
            {"time": float(time_seconds), "value": float(strength)}
            for time_seconds, strength in strength_keys
        ],
    }
    payload["direction_keys"].sort(key=lambda item: item["time"])
    payload["strength_keys"].sort(key=lambda item: item["time"])
    return json.dumps(payload, indent=indent)


__all__ = ["build_keyframe_groom_wind_json"]

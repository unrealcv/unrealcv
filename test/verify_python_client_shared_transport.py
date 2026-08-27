"""Manual, runtime verification of automatic Shared Memory Python APIs."""

import json
import sys
from functools import wraps

import numpy as np

sys.path.insert(0, "client/python")

from unrealcv import SharedCommand, UnrealCv_API, UnrealCvPlusAPI


def main():
    api = UnrealCv_API(
        port=9000,
        ip="127.0.0.1",
        resolution=(640, 480),
        mode="tcp",
    )
    calls = []
    original_request = api.client.request

    @wraps(original_request)
    def record_request(message, *args, **kwargs):
        calls.append({
            "message": message,
            "is_shared_command": isinstance(message, SharedCommand),
            "response_format": getattr(message, "response_format", None),
        })
        return original_request(message, *args, **kwargs)

    api.client.request = record_request

    cases = [
        ("get_image(lit,bmp)", lambda: api.get_image(0, "lit", "bmp")),
        ("get_image(normal,png)", lambda: api.get_image(0, "normal", "png")),
        ("get_image(object_mask,bmp)", lambda: api.get_image(0, "object_mask", "bmp")),
        ("get_depth()", lambda: api.get_depth(0)),
        ("get_image_multicam()", lambda: api.get_image_multicam([0, 1], "lit", "bmp", False)),
        ("get_image_multimodal()", lambda: api.get_image_multimodal(0)),
        ("get_img_batch()", lambda: api.get_img_batch({
            0: {
                "lit": {"mode": "bmp", "inverse": False},
                "depth": {"mode": "npy", "inverse": False},
            }
        })),
    ]

    results = []
    for label, function in cases:
        before = len(calls)
        value = function()
        captured = calls[before:]
        messages = []
        for call in captured:
            message = call["message"]
            if isinstance(message, list):
                messages.extend(message)
            else:
                messages.append(message)
        shared_messages = [str(message) for message in messages if "_shared" in str(message)]
        if isinstance(value, dict):
            shapes = {
                f"{camera_id}:{modality}": {
                    "shape": list(item["img"].shape),
                    "dtype": str(item["img"].dtype),
                }
                for camera_id, camera in value.items()
                for modality, item in camera.items()
            }
        elif isinstance(value, list):
            shapes = [
                {"shape": list(item.shape), "dtype": str(item.dtype)}
                for item in value
            ]
        else:
            shapes = {"shape": list(value.shape), "dtype": str(value.dtype)}
        result = {
            "python_function": label,
            "shared_selected": bool(shared_messages),
            "selected_commands": shared_messages,
            "return": shapes,
        }
        results.append(result)
        print(json.dumps(result, ensure_ascii=False))

    if not all(item["shared_selected"] for item in results):
        raise SystemExit("At least one image API did not select Shared Memory")

    plus_api = UnrealCvPlusAPI.__new__(UnrealCvPlusAPI)
    plus_api.client = api.client
    plus_api.api_version = api.api_version
    plus_api.decoder = api.decoder
    plus_api.checker = api.checker
    plus_api.cam = api.cam
    plus_api.obj_dict = api.obj_dict

    plus_cases = [
        (
            "get_scene_occupancy(profile='lingo_vis', method='mesh')",
            lambda: plus_api.get_scene_occupancy(
                profile="lingo_vis",
                method="mesh",
            ),
        ),
        (
            "get_scene_occupancy_region(min_m=(-5,-1,-5), max_m=(5,3,5), voxel_size_m=1, method='mesh')",
            lambda: plus_api.get_scene_occupancy_region(
                min_m=(-5, -1, -5),
                max_m=(5, 3, 5),
                voxel_size_m=1,
                method="mesh",
            ),
        ),
        (
            "get_camera_panoramic_frame(cam_id=0, width=64, height=32)",
            lambda: plus_api.get_camera_panoramic_frame(0, width=64, height=32),
        ),
        (
            "get_camera_panoramic_normal_frame(cam_id=0, width=64, height=32)",
            lambda: plus_api.get_camera_panoramic_normal_frame(0, width=64, height=32),
        ),
        (
            "get_camera_panoramic_mask_frame(cam_id=0, width=64, height=32)",
            lambda: plus_api.get_camera_panoramic_mask_frame(0, width=64, height=32),
        ),
        (
            "get_camera_panoramic_depth_frame(cam_id=0, width=64, height=32)",
            lambda: plus_api.get_camera_panoramic_depth_frame(0, width=64, height=32),
        ),
        (
            "get_camera_mqrc_lit_frame(cam_id=0)",
            lambda: plus_api.get_camera_mqrc_lit_frame(0),
        ),
        (
            "get_camera_lidar_frame(cam_id=0)",
            lambda: plus_api.get_camera_lidar_frame(0),
        ),
        (
            "get_camera_mqrc_panoramic_frame(cam_id=0, width=64, height=32)",
            lambda: plus_api.get_camera_mqrc_panoramic_frame(0, 64, 32),
        ),
        (
            "get_camera_mqrc_panoramic_frame(cam_id=0, width=64, height=32, face_resolution=16)",
            lambda: plus_api.get_camera_mqrc_panoramic_frame(0, 64, 32, 16),
        ),
    ]
    for label, function in plus_cases:
        before = len(calls)
        value = function()
        captured = calls[before:]
        messages = []
        for call in captured:
            message = call["message"]
            messages.extend(message if isinstance(message, list) else [message])
        shared_messages = [str(message) for message in messages if "_shared" in str(message)]
        result = {
            "python_function": label,
            "shared_selected": bool(shared_messages),
            "selected_commands": shared_messages,
            "return": {"shape": list(value.shape), "dtype": str(value.dtype)},
        }
        print(json.dumps(result, ensure_ascii=False))
        if not result["shared_selected"]:
            raise SystemExit(f"{label} did not select Shared Memory")

    api.client.disconnect()


if __name__ == "__main__":
    main()

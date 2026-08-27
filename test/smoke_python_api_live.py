"""Unified live smoke test for the public Python UnrealCV APIs.

The runner covers methods declared by both ``UnrealCv_API`` and
``UnrealCvPlusAPI``.  It records command construction separately from live
execution because some high-level helpers do not expose ``return_cmd``.
"""

import argparse
import csv
import inspect
import json
import tempfile
from pathlib import Path

from unrealcv.api import UnrealCv_API
from unrealcv.plus_api import UnrealCvPlusAPI


def _sample(parameter, temp_dir):
    name = parameter.name
    if name in {"cam_id", "camera_id"}:
        return 0
    if name in {"obj", "object_name", "actor_name"}:
        return "UnrealcvPawn_2147470225"
    if name in {"objects", "cam_ids"}:
        return ["UnrealcvPawn_2147470225"] if name == "objects" else [0]
    if name == "object_filter":
        return "visible"
    if name in {"path", "filename", "foreground_path", "background_path"}:
        return str(temp_dir / f"{name}.png")
    if name in {"asset_path", "package_path", "pak_file_path"}:
        return "/Game/DoesNotExist"
    if name in {"loc", "pos_now", "pos_exp"}:
        return [0.0, 0.0, 0.0]
    if name in {"rot", "pose"}:
        return [0.0, 0.0, 0.0]
    if name == "color":
        return [255, 0, 0]
    if name in {"object_mask", "object_masks"}:
        return None
    if name == "cam_info":
        return [(0, "lit", "bmp")]
    if name in {"viewmode", "viewmodes"}:
        return "lit" if name == "viewmode" else ["lit"]
    if name in {"mode", "modes"}:
        return "png" if name == "mode" else ["png"]
    if name in {"key", "focus_mode", "projection_type", "method", "profile"}:
        return "manual" if name == "focus_mode" else "bounds"
    if name == "map_name":
        return "SuburbNeighborhood_Day"
    if name == "class_name":
        return "StaticMeshActor"
    if name == "obj_name":
        return "SmokeObject"
    if name == "request_id":
        return "missing-request"
    if name in {"origin_cm", "min_m", "max_m"}:
        return [0.0, 0.0, 0.0] if name == "origin_cm" else ([-1.0] * 3 if name == "min_m" else [1.0] * 3)
    if name in {"enabled", "paused", "newest", "syns", "inverse", "show", "annotate", "batch", "box", "rpy", "return_dict", "include_dynamic", "force_rescan", "apply_physical_exposure", "smoothing_enabled", "crop_overscan", "scale_resolution_with_overscan"}:
        return False if name in {"paused", "inverse", "show", "include_dynamic", "force_rescan", "apply_physical_exposure", "smoothing_enabled", "crop_overscan", "scale_resolution_with_overscan"} else True
    if name in {"width", "height"}:
        return 64
    if name in {"n", "threshold", "fps", "duration_seconds", "warmup_frames", "blade_count", "uint_arg1", "uint_arg2", "uint_arg3"}:
        return 1
    if name in {"time_dilation", "dilation", "fov", "focus_distance_cm", "distance_cm", "aspect_ratio"}:
        return 1.0
    if name.startswith("uint_"):
        return 1
    if name.startswith("bool_"):
        return False
    if name.startswith("float_") or name.endswith("_mm") or name.endswith("_cm"):
        return 1.0
    if name.startswith("str_") or name in {"value", "category", "mount_point", "mode_name"}:
        return "0"
    if parameter.default is not inspect.Parameter.empty:
        return parameter.default
    return 0


def _required_arguments(method, temp_dir):
    signature = inspect.signature(method)
    args = []
    for parameter in signature.parameters.values():
        if parameter.kind not in (parameter.POSITIONAL_ONLY, parameter.POSITIONAL_OR_KEYWORD):
            continue
        if parameter.name in {"return_cmd", "timeout", "show", "inverse", "newest", "syns", "return_dict", "batch", "annotate", "box", "rpy", "include_dynamic"} and parameter.default is not inspect.Parameter.empty:
            continue
        value = _sample(parameter, temp_dir)
        if parameter.name == "max_m":
            value = [1.0, 1.0, 1.0]
        args.append(value)
    return args, signature


def _make_plus(base_api):
    plus = UnrealCvPlusAPI.__new__(UnrealCvPlusAPI)
    plus.__dict__.update(base_api.__dict__)
    return plus


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--output", type=Path, default=Path("test/python_api_live_smoke.json"))
    parser.add_argument("--csv", type=Path, default=Path("test/python_api_live_smoke.csv"))
    args = parser.parse_args()

    base = UnrealCv_API(args.port, args.host, (640, 480))
    apis = [("UnrealCv_API", base), ("UnrealCvPlusAPI", _make_plus(base))]
    rows = []
    with tempfile.TemporaryDirectory(prefix="unrealcv-api-smoke-") as temp:
        temp_dir = Path(temp)
        for class_name, api in apis:
            declared = dict(vars(UnrealCv_API))
            if class_name == "UnrealCvPlusAPI":
                declared.update(vars(UnrealCvPlusAPI))
            for name, member in sorted(declared.items()):
                if name.startswith("_") or not inspect.isfunction(member):
                    continue
                if not (name.startswith(("get_", "set_", "save_", "spawn_", "start_", "stop_", "is_", "annotate_", "clear_", "mount_", "unmount_", "register_", "scan_", "move_", "destroy_", "config_", "batch_", "supports_", "capture_"))):
                    continue
                method = getattr(api, name)
                call_args, signature = _required_arguments(method, temp_dir)
                row = {"class": class_name, "function": name, "signature": str(signature)}
                if "return_cmd" in signature.parameters:
                    try:
                        command = method(*call_args, return_cmd=True)
                        row.update(command_status="PASS", command=repr(command))
                    except Exception as exc:
                        row.update(command_status="FAIL", command_error=f"{type(exc).__name__}: {exc}")
                else:
                    row["command_status"] = "N/A"
                is_mutating = name.startswith(("set_", "save_", "spawn_", "start_", "stop_", "annotate_", "clear_", "mount_", "unmount_", "register_", "scan_", "move_", "destroy_", "config_", "capture_"))
                if is_mutating:
                    row.update(live_status="SKIP_MUTATION", live_error="Mutation is covered by command construction; use an explicit workflow to execute it.")
                elif (name in {"batch_cmd", "get_image", "get_depth", "get_optical_flow", "get_image_multicam", "get_image_multimodal", "get_img_batch"}
                      or "shared" in name or name.startswith("capture_")):
                    row.update(live_status="SKIP_SPECIAL_TRANSPORT", live_error="Binary, shared-memory, batch, or capture API is covered by its dedicated transport smoke test.")
                else:
                    try:
                        kwargs = {"timeout": 5} if "timeout" in signature.parameters else {}
                        result = method(*call_args, **kwargs)
                        row.update(live_status="PASS", live_result=repr(result)[:500])
                    except Exception as exc:  # pragma: no cover - live server diagnostics
                        row.update(live_status="FAIL", live_error=f"{type(exc).__name__}: {exc}")
                rows.append(row)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(rows, indent=2), encoding="utf-8")
    fields = ["class", "function", "signature", "command_status", "command", "command_error", "live_status", "live_result", "live_error"]
    with args.csv.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)
    print(json.dumps({
        "functions": len(rows),
        "command_pass": sum(row["command_status"] == "PASS" for row in rows),
        "live_pass": sum(row.get("live_status") == "PASS" for row in rows),
        "live_fail": sum(row.get("live_status") == "FAIL" for row in rows),
        "live_skipped_mutation": sum(row.get("live_status") == "SKIP_MUTATION" for row in rows),
        "live_skipped_special": sum(row.get("live_status") == "SKIP_SPECIAL_TRANSPORT" for row in rows),
        "output": str(args.output),
        "csv": str(args.csv),
    }, indent=2))


if __name__ == "__main__":
    main()

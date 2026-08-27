"""Live smoke test for the explicit UnrealCV Plus Python API signatures."""

import argparse
import inspect
import json
import tempfile
from pathlib import Path

from unrealcv.plus_api import UnrealCvPlusAPI


def sample_for(parameter, output_dir):
    name = parameter.name
    if name == "cam_id":
        return 0
    if name in {"object_name", "actor_name"}:
        return "UnrealcvPawn_2147482376"
    if name == "object_filter":
        return "visible"
    if name in {"path", "foreground_path", "background_path"}:
        return str(output_dir / f"{name}.png")
    if name == "request_id":
        return "missing-request"
    if name.startswith("str_"):
        return "0"
    if name.startswith("float_"):
        return 0.0
    if name.startswith("uint_"):
        return 1
    if name.startswith("bool_"):
        return 0
    if name == "value":
        return "{}"
    if parameter.default is None:
        return None
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=9000)
    parser.add_argument("--output", type=Path, default=Path("test/plus_api_live_smoke.json"))
    args = parser.parse_args()

    api = UnrealCvPlusAPI(args.port, args.host, (640, 480))
    results = []
    with tempfile.TemporaryDirectory(prefix="unrealcv-plus-smoke-") as temp_dir:
        output_dir = Path(temp_dir)
        own_methods = {
            name: getattr(api, name) for name, member in vars(UnrealCvPlusAPI).items()
            if inspect.isfunction(member)
        }
        for name, method in sorted(own_methods.items()):
            if not (name.startswith("get_") or name.startswith("set_")):
                continue
            signature = inspect.signature(method)
            if "return_cmd" not in signature.parameters or "timeout" not in signature.parameters:
                continue
            positional = [
                parameter for parameter in signature.parameters.values()
                if parameter.name not in {"return_cmd", "timeout"}
                and parameter.kind in (parameter.POSITIONAL_ONLY,
                                       parameter.POSITIONAL_OR_KEYWORD)
            ]
            call_args = [sample_for(parameter, output_dir) for parameter in positional]
            row = {"function": name, "signature": str(signature)}
            try:
                row["command"] = method(*call_args, return_cmd=True)
                row["command_status"] = "PASS"
            except Exception as exc:  # pragma: no cover - live server diagnostics
                row["command_status"] = "FAIL"
                row["command_error"] = f"{type(exc).__name__}: {exc}"
            if name.startswith("get_"):
                try:
                    row["live_result"] = repr(method(*call_args, timeout=5))[:500]
                    row["live_status"] = "PASS"
                except Exception as exc:  # pragma: no cover - live server diagnostics
                    row["live_status"] = "FAIL"
                    row["live_error"] = f"{type(exc).__name__}: {exc}"
            results.append(row)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(results, indent=2), encoding="utf-8")
    print(json.dumps({
        "functions": len(results),
        "command_pass": sum(row["command_status"] == "PASS" for row in results),
        "live_getters": sum("live_status" in row for row in results),
        "live_pass": sum(row.get("live_status") == "PASS" for row in results),
        "output": str(args.output),
    }, indent=2))


if __name__ == "__main__":
    main()

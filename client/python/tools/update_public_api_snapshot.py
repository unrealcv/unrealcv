import argparse
import ast
import json
from pathlib import Path


MODULES = ["api.py", "automation.py", "launcher.py"]


def _collect_public_names_from_module(path: Path) -> set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8", errors="ignore"), filename=str(path))
    exported: set[str] = set()
    for node in tree.body:
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            if not node.name.startswith("_"):
                exported.add(node.name)
        elif isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and not target.id.startswith("_"):
                    exported.add(target.id)
    return exported


def build_snapshot(repo_root: Path) -> dict[str, object]:
    package_dir = repo_root / "client/python/unrealcv"
    names: set[str] = set()

    init_names = _collect_public_names_from_module(package_dir / "__init__.py")
    names.update(init_names)

    for module in MODULES:
        names.update(_collect_public_names_from_module(package_dir / module))

    names.discard("annotations")

    return {
        "schema_version": 1,
        "source": "ast-static-analysis",
        "symbol_count": len(names),
        "symbols": sorted(names),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Update or validate UnrealCV Python public API snapshot")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument(
        "--snapshot",
        default="client/python/unrealcv/public_api_snapshot.json",
        help="Path to snapshot json file, relative to repo root",
    )
    parser.add_argument("--check", action="store_true", help="Check mode: fail if snapshot is outdated")
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    snapshot_path = (repo_root / args.snapshot).resolve()
    snapshot_path.parent.mkdir(parents=True, exist_ok=True)

    generated = build_snapshot(repo_root)

    if args.check and snapshot_path.exists():
        current = json.loads(snapshot_path.read_text(encoding="utf-8"))
        if current != generated:
            print("Public API snapshot is out of date. Run update_public_api_snapshot.py without --check.")
            return 1
        print("Public API snapshot is up to date")
        return 0

    snapshot_path.write_text(json.dumps(generated, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote API snapshot with {generated['symbol_count']} symbols to {snapshot_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

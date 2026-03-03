import ast
import json
from pathlib import Path


MODULES = ["api.py", "automation.py", "launcher.py"]


def _collect_public_names(path: Path) -> set[str]:
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


def _current_symbols(repo_root: Path) -> set[str]:
    package_dir = repo_root / "client/python/unrealcv"
    symbols = _collect_public_names(package_dir / "__init__.py")
    for module in MODULES:
        symbols.update(_collect_public_names(package_dir / module))
    symbols.discard("annotations")
    return symbols


def test_public_api_snapshot_has_no_removed_symbols():
    repo_root = Path(__file__).resolve().parents[2]
    snapshot_path = repo_root / "client/python/unrealcv/public_api_snapshot.json"
    snapshot = json.loads(snapshot_path.read_text(encoding="utf-8"))

    expected_symbols = set(snapshot["symbols"])
    current_symbols = _current_symbols(repo_root)

    removed = sorted(expected_symbols - current_symbols)
    assert not removed, (
        "Public API symbols were removed. "
        "If this is intentional, document the breaking change and refresh snapshot. "
        f"Removed: {removed}"
    )

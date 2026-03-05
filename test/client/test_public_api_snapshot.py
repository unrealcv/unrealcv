import ast
import json
from pathlib import Path
from typing import Set


MODULES = ["api.py", "automation.py", "launcher.py"]


def _collect_public_names(path: Path) -> Set[str]:
    tree = ast.parse(path.read_text(encoding="utf-8", errors="ignore"), filename=str(path))
    exported: Set[str] = set()
    for node in tree.body:
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            if not node.name.startswith("_"):
                exported.add(node.name)
        elif isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and not target.id.startswith("_"):
                    exported.add(target.id)
    return exported


def _current_symbols(repo_root: Path) -> Set[str]:
    package_dir = repo_root / "client/python/unrealcv"
    symbols = _collect_public_names(package_dir / "__init__.py")
    for module in MODULES:
        symbols.update(_collect_public_names(package_dir / module))
    symbols.discard("annotations")
    symbols.discard("h")
    return symbols


def _has_import_star(init_path: Path, module_name: str) -> bool:
    tree = ast.parse(init_path.read_text(encoding="utf-8", errors="ignore"), filename=str(init_path))
    for node in tree.body:
        if not isinstance(node, ast.ImportFrom):
            continue
        if node.level == 1 and node.module == module_name and any(alias.name == "*" for alias in node.names):
            return True
    return False


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


def test_package_init_reexports_public_modules():
    repo_root = Path(__file__).resolve().parents[2]
    init_path = repo_root / "client/python/unrealcv/__init__.py"

    missing = sorted(module.replace(".py", "") for module in MODULES if not _has_import_star(init_path, module.replace(".py", "")))
    assert not missing, (
        "Package __init__.py must re-export public modules with 'from .<module> import *'. "
        f"Missing re-exports: {missing}"
    )

import importlib.util
import json
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]


def _load_generator_module():
    path = REPO_ROOT / "tools/command_schema/generate_schema.py"
    spec = importlib.util.spec_from_file_location("command_schema_generator", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def test_committed_schema_matches_all_production_bindings():
    generator = _load_generator_module()
    generated = generator.generate_schema(REPO_ROOT)
    committed = json.loads(
        (REPO_ROOT / "docs/reference/command_schema.json").read_text(encoding="utf-8")
    )
    assert committed == generated


def test_generated_rst_mentions_every_schema_command():
    schema = json.loads(
        (REPO_ROOT / "docs/reference/command_schema.json").read_text(encoding="utf-8")
    )
    docs = (REPO_ROOT / "docs/reference/commands_generated.rst.txt").read_text(
        encoding="utf-8"
    )
    missing = [item["command"] for item in schema["commands"] if f"``{item['command']}``" not in docs]
    assert not missing


def test_workflow_registry_parser_covers_schema_help_output():
    path = REPO_ROOT / "workflow/command_coverage.py"
    spec = importlib.util.spec_from_file_location("workflow_command_coverage", path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)

    schema_path = REPO_ROOT / "docs/reference/command_schema.json"
    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    simulated_help = "\n".join(
        line
        for item in schema["commands"]
        if item.get("availability", "runtime") == "runtime"
        for line in (item["command"], item.get("description", ""))
    )
    coverage = module.compare_registry_with_schema(simulated_help, schema_path)
    assert coverage["missing"] == set()
    assert module.has_registered_route(
        coverage["registered"], "vget /camera/[uint]/location"
    )
    assert not module.has_registered_route(
        coverage["registered"], "vget /cameras_CID"
    )

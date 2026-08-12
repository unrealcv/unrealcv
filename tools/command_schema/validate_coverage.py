import argparse
import json
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate generated docs and workflow command coverage")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--schema", default="docs/reference/command_schema.json")
    parser.add_argument("--strict", action="store_true", help="Fail when any coverage gate is incomplete")
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    schema = json.loads((repo_root / args.schema).read_text(encoding="utf-8"))
    schema_commands = {item["command"] for item in schema.get("commands", [])}

    generated_docs = (repo_root / "docs/reference/commands_generated.rst.txt").read_text(
        encoding="utf-8", errors="ignore"
    )
    handwritten_docs = (repo_root / "docs/reference/commands.rst").read_text(
        encoding="utf-8", errors="ignore"
    )
    missing_generated_docs = sorted(
        command for command in schema_commands if f"``{command}``" not in generated_docs
    )
    handwritten_count = sum(command in handwritten_docs for command in schema_commands)

    workflow_module = (repo_root / "workflow/command_coverage.py").read_text(encoding="utf-8")
    workflow_runner = (repo_root / "workflow/test_runner.py").read_text(encoding="utf-8")
    workflow_covered = (
        "compare_registry_with_schema" in workflow_module
        and "compare_registry_with_schema(" in workflow_runner
        and "Command Registry Coverage" in workflow_runner
    )

    python_docs = (repo_root / "docs/reference/python_api.rst").read_text(encoding="utf-8")
    conf_text = (repo_root / "docs/conf.py").read_text(encoding="utf-8")
    python_autodoc_covered = (
        ".. automodule:: unrealcv.api" in python_docs
        and ":members:" in python_docs
        and "'client', 'python'" in conf_text
    )

    total = len(schema_commands)
    print("Command and API coverage report")
    print(f"- generated_command_docs: {total - len(missing_generated_docs)}/{total}")
    print(f"- hand_written_command_mentions: {handwritten_count}/{total}")
    print(f"- workflow_registry_contract: {'yes' if workflow_covered else 'no'}")
    print(f"- local_python_api_autodoc: {'yes' if python_autodoc_covered else 'no'}")

    failures = []
    if missing_generated_docs:
        failures.append(f"generated docs missing {len(missing_generated_docs)} commands")
    if not workflow_covered:
        failures.append("workflow registry contract is not wired into test_runner.py")
    if not python_autodoc_covered:
        failures.append("Python API autodoc is not configured to import the local client")
    if args.strict and failures:
        for failure in failures:
            print(f"ERROR: {failure}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

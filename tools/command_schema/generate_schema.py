import argparse
import json
import re
from pathlib import Path

BIND_RE = re.compile(
    r"BindCommand\s*\(\s*(?:TEXT\()?(?P<quote>\"|')(?P<cmd>.*?)(?P=quote)\)?\s*,",
    re.DOTALL,
)


def infer_category(command: str) -> str:
    parts = command.split()
    if len(parts) < 2:
        return "misc"
    path = parts[1]
    segments = [s for s in path.split("/") if s]
    if not segments:
        return "misc"
    return segments[0]


def extract_commands_from_file(path: Path) -> list[dict]:
    text = path.read_text(encoding="utf-8", errors="ignore")
    items: list[dict] = []
    for match in BIND_RE.finditer(text):
        command = " ".join(match.group("cmd").split())
        line = text.count("\n", 0, match.start()) + 1
        items.append(
            {
                "command": command,
                "category": infer_category(command),
                "source": str(path).replace("\\", "/"),
                "line": line,
            }
        )
    return items


def generate_schema(repo_root: Path) -> dict:
    command_files = sorted((repo_root / "Source/UnrealCV/Private/Commands").glob("*.cpp"))
    extracted: list[dict] = []
    for file_path in command_files:
        extracted.extend(extract_commands_from_file(file_path))

    unique: dict[str, dict] = {}
    for item in extracted:
        unique.setdefault(item["command"], item)

    commands = sorted(unique.values(), key=lambda x: x["command"])
    return {
        "schema_version": 1,
        "generated_from": "Source/UnrealCV/Private/Commands/*.cpp",
        "command_count": len(commands),
        "commands": commands,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate UnrealCV command schema JSON from C++ registrations")
    parser.add_argument("--repo-root", default=".", help="Repository root path")
    parser.add_argument(
        "--output",
        default="docs/reference/command_schema.json",
        help="Output schema file path (relative to repo root)",
    )
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    output_path = (repo_root / args.output).resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)

    schema = generate_schema(repo_root)
    output_path.write_text(json.dumps(schema, indent=2, sort_keys=False) + "\n", encoding="utf-8")
    print(f"Wrote {schema['command_count']} commands to {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

import argparse
import json
import re
from pathlib import Path

CLIENT_CMD_RE = re.compile(r"\b(vget|vset|vrun|vexec|vbp)\s+/[^\s'\"]+")


def normalize(command: str) -> str:
    text = " ".join(command.split()).strip()
    text = re.sub(r"\[[^\]]+\]", "[arg]", text)
    text = re.sub(r"\{[^\}]+\}", "arg", text)
    text = re.sub(r"/\d+", "/[arg]", text)
    return text


def path_token(command: str) -> str:
    parts = command.split()
    if len(parts) < 2:
        return normalize(command)
    return normalize(f"{parts[0]} {parts[1]}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate docs/client coverage against generated command schema")
    parser.add_argument("--repo-root", default=".")
    parser.add_argument("--schema", default="docs/reference/command_schema.json")
    parser.add_argument("--strict", action="store_true", help="Fail if there are uncovered command paths")
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    schema_path = (repo_root / args.schema).resolve()

    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    schema_commands = [normalize(item["command"]) for item in schema.get("commands", [])]

    docs_text = (repo_root / "docs/reference/commands.rst").read_text(encoding="utf-8", errors="ignore")
    normalized_docs_text = normalize(docs_text)
    docs_hits = [cmd for cmd in schema_commands if cmd in normalized_docs_text]

    client_text = "\n".join(
        p.read_text(encoding="utf-8", errors="ignore")
        for p in sorted((repo_root / "client/python/unrealcv").glob("*.py"))
    )
    client_literals = {normalize(m.group(0)) for m in CLIENT_CMD_RE.finditer(client_text)}

    schema_path_tokens = {
        path_token(cmd)
        for cmd in schema_commands
        if len(cmd.split()) >= 2 and cmd.split()[1].startswith("/")
    }
    client_path_tokens = {path_token(cmd) for cmd in client_literals}
    missing_in_client = sorted(schema_path_tokens - client_path_tokens)

    docs_ratio = (len(docs_hits) / len(schema_commands)) if schema_commands else 1.0
    client_ratio = (
        (len(schema_path_tokens) - len(missing_in_client)) / len(schema_path_tokens)
        if schema_path_tokens
        else 1.0
    )

    print("Command schema coverage report")
    print(f"- total_commands: {len(schema_commands)}")
    print(f"- docs_exact_matches: {len(docs_hits)} ({docs_ratio:.1%})")
    print(f"- client_path_coverage: {len(schema_path_tokens) - len(missing_in_client)}/{len(schema_path_tokens)} ({client_ratio:.1%})")

    if missing_in_client:
        print("- missing_client_paths:")
        for item in missing_in_client[:50]:
            print(f"  - {item}")

    if args.strict and missing_in_client:
        print("Coverage validation failed in strict mode: missing client command paths detected")
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

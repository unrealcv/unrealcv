import argparse
import ast
import json
import re
from collections import defaultdict
from pathlib import Path


COMMAND_VERBS = {"vbp", "vcmd", "vexec", "vget", "vrun", "vset"}
CATEGORY_TITLES = {
    "action": "Action",
    "alias": "Aliases and engine commands",
    "camera": "Camera",
    "cameras": "Camera collection",
    "level": "Level",
    "misc": "Miscellaneous",
    "object": "Object",
    "objects": "Object collection",
    "persistent_level": "Persistent level",
    "scene": "Scene",
    "screenshot": "Screenshot",
    "unrealcv": "Plugin",
    "viewmode": "View mode",
}


def infer_category(command: str) -> str:
    parts = command.split()
    if len(parts) < 2:
        return "alias" if parts and parts[0] in {"vrun", "vexec", "vbp"} else "misc"
    path = parts[1]
    if not path.startswith("/"):
        return "alias" if parts[0] in {"vrun", "vexec", "vbp"} else "misc"
    segments = [segment for segment in path.split("/") if segment]
    return segments[0] if segments else "misc"


def _strip_comments(text: str) -> str:
    output = []
    index = 0
    state = "code"
    quote = ""
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char in {'"', "'"}:
                state = "string"
                quote = char
                output.append(char)
            elif char == "/" and next_char == "/":
                state = "line_comment"
                output.extend("  ")
                index += 1
            elif char == "/" and next_char == "*":
                state = "block_comment"
                output.extend("  ")
                index += 1
            else:
                output.append(char)
        elif state == "string":
            output.append(char)
            if char == "\\" and next_char:
                output.append(next_char)
                index += 1
            elif char == quote:
                state = "code"
        elif state == "line_comment":
            output.append("\n" if char == "\n" else " ")
            if char == "\n":
                state = "code"
        else:
            output.append("\n" if char == "\n" else " ")
            if char == "*" and next_char == "/":
                output.append(" ")
                index += 1
                state = "code"
        index += 1
    return "".join(output)


def _find_matching_paren(text: str, open_index: int) -> int:
    depth = 0
    quote = ""
    index = open_index
    while index < len(text):
        char = text[index]
        if quote:
            if char == "\\":
                index += 2
                continue
            if char == quote:
                quote = ""
        elif char in {'"', "'"}:
            quote = char
        elif char == "(":
            depth += 1
        elif char == ")":
            depth -= 1
            if depth == 0:
                return index
        index += 1
    raise ValueError("Unclosed BindCommand call")


def _split_top_level_args(text: str) -> list[str]:
    args = []
    start = 0
    depth = 0
    quote = ""
    index = 0
    while index < len(text):
        char = text[index]
        if quote:
            if char == "\\":
                index += 2
                continue
            if char == quote:
                quote = ""
        elif char in {'"', "'"}:
            quote = char
        elif char in "([{":
            depth += 1
        elif char in ")]}":
            depth -= 1
        elif char == "," and depth == 0:
            args.append(text[start:index].strip())
            start = index + 1
        index += 1
    args.append(text[start:].strip())
    return args


def _decode_cpp_string(expression: str) -> str | None:
    expression = expression.strip()
    text_match = re.fullmatch(r"TEXT\s*\(\s*(.+)\s*\)", expression, re.DOTALL)
    if text_match:
        expression = text_match.group(1).strip()
    string_tokens = re.findall(r'"(?:\\.|[^"\\])*"', expression, re.DOTALL)
    if not string_tokens or re.sub(r'"(?:\\.|[^"\\])*"', "", expression).strip():
        return None
    try:
        return "".join(ast.literal_eval(token) for token in string_tokens)
    except (SyntaxError, ValueError):
        return None


def _resolve_description(expression: str, prefix: str) -> str:
    literal = _decode_cpp_string(expression)
    if literal is not None:
        return " ".join(literal.split())
    identifier_match = re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", expression.strip())
    if not identifier_match:
        return ""
    identifier = re.escape(identifier_match.group(0))
    assignments = list(
        re.finditer(
            rf"\b{identifier}\s*=\s*(?P<value>(?:TEXT\s*\(\s*)?(?:\"(?:\\.|[^\"\\])*\"\s*)+(?:\))?)\s*;",
            prefix,
            re.DOTALL,
        )
    )
    if not assignments:
        return ""
    return " ".join((_decode_cpp_string(assignments[-1].group("value")) or "").split())


def _availability_at(text: str, position: int) -> str:
    editor_depth = 0
    conditional_stack = []
    for line in text[:position].splitlines():
        stripped = line.strip()
        if re.match(r"#\s*(if|ifdef)\b", stripped):
            is_editor = bool(re.search(r"\bWITH_EDITOR\b", stripped)) and not bool(
                re.search(r"!\s*WITH_EDITOR", stripped)
            )
            conditional_stack.append(is_editor)
            editor_depth += int(is_editor)
        elif re.match(r"#\s*endif\b", stripped) and conditional_stack:
            editor_depth -= int(conditional_stack.pop())
    return "editor" if editor_depth else "runtime"


def extract_commands_from_file(path: Path, repo_root: Path) -> list[dict]:
    original_text = path.read_text(encoding="utf-8", errors="ignore")
    text = _strip_comments(original_text)
    items = []
    relative_source = path.relative_to(repo_root).as_posix()
    for match in re.finditer(r"\bBindCommand\s*\(", text):
        open_index = text.find("(", match.start())
        close_index = _find_matching_paren(text, open_index)
        args = _split_top_level_args(text[open_index + 1 : close_index])
        if len(args) < 3:
            continue
        command = _decode_cpp_string(args[0])
        if not command:
            continue
        command = " ".join(command.split())
        if command.split()[0] not in COMMAND_VERBS:
            continue
        items.append(
            {
                "command": command,
                "category": infer_category(command),
                "description": _resolve_description(args[2], text[: match.start()]),
                "availability": _availability_at(text, match.start()),
                "source": relative_source,
                "line": original_text.count("\n", 0, match.start()) + 1,
            }
        )
    return items


def generate_schema(repo_root: Path) -> dict:
    private_root = repo_root / "Source/UnrealCV/Private"
    command_files = sorted(
        path for path in private_root.rglob("*.cpp") if "Tests" not in path.parts
    )
    extracted = []
    for file_path in command_files:
        extracted.extend(extract_commands_from_file(file_path, repo_root))

    unique = {}
    for item in extracted:
        unique.setdefault(item["command"], item)
    commands = sorted(unique.values(), key=lambda item: item["command"])
    return {
        "schema_version": 2,
        "generated_from": "Source/UnrealCV/Private/**/*.cpp (excluding Tests)",
        "command_count": len(commands),
        "commands": commands,
    }


def render_rst(schema: dict) -> str:
    lines = [
        ".. This file is generated by tools/command_schema/generate_schema.py.",
        ".. Edit the C++ registration/help text or docs/reference/commands.rst instead.",
        "",
        "This index lists every command registered by the current C++ server. The",
        "hand-written sections above remain the place for examples, compatibility notes,",
        "and longer explanations.",
        "",
    ]
    grouped = defaultdict(list)
    for command in schema["commands"]:
        grouped[command["category"]].append(command)
    for category in sorted(grouped):
        title = CATEGORY_TITLES.get(category, category.replace("_", " ").title())
        lines.extend([title, "~" * len(title), ""])
        for item in grouped[category]:
            lines.append(f"``{item['command']}``")
            description = item["description"] or "No command help text is registered."
            lines.append(f"    {description}")
            if item["availability"] != "runtime":
                lines.append(f"    Availability: {item['availability']} builds only.")
            lines.append(f"    Source: :file:`{item['source']}:{item['line']}`")
            lines.append("")
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate UnrealCV command schema and RST index")
    parser.add_argument("--repo-root", default=".", help="Repository root path")
    parser.add_argument(
        "--output",
        default="docs/reference/command_schema.json",
        help="Output schema path relative to the repository root",
    )
    parser.add_argument(
        "--rst-output",
        default="docs/reference/commands_generated.rst.txt",
        help="Generated RST command index path relative to the repository root",
    )
    args = parser.parse_args()

    repo_root = Path(args.repo_root).resolve()
    output_path = (repo_root / args.output).resolve()
    rst_output_path = (repo_root / args.rst_output).resolve()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    rst_output_path.parent.mkdir(parents=True, exist_ok=True)

    schema = generate_schema(repo_root)
    output_path.write_text(json.dumps(schema, indent=2) + "\n", encoding="utf-8")
    rst_output_path.write_text(render_rst(schema), encoding="utf-8")
    print(f"Wrote {schema['command_count']} commands to {output_path} and {rst_output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

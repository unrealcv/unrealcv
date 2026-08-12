"""Runtime command-registry coverage checks shared by the workflow harness."""

import json
from pathlib import Path


COMMAND_VERBS = ("vbp ", "vcmd ", "vexec ", "vget ", "vrun ", "vset ")


def load_schema_commands(schema_path: Path, availability="runtime"):
    schema = json.loads(schema_path.read_text(encoding="utf-8"))
    return {
        item["command"]
        for item in schema.get("commands", [])
        if item.get("availability", "runtime") == availability
    }


def parse_help_commands(help_response):
    if isinstance(help_response, bytes):
        help_response = help_response.decode("utf-8", errors="replace")
    return {
        line.strip()
        for line in str(help_response).splitlines()
        if line.strip().startswith(COMMAND_VERBS)
    }


def command_route(command):
    parts = command.split()
    if len(parts) >= 2 and parts[1].startswith("/"):
        return " ".join(parts[:2])
    return " ".join(parts)


def has_registered_route(registered_commands, command):
    route = command_route(command)
    return any(command_route(item) == route for item in registered_commands)


def compare_registry_with_schema(help_response, schema_path: Path, availability="runtime"):
    expected = load_schema_commands(schema_path, availability=availability)
    registered = parse_help_commands(help_response)
    return {
        "expected": expected,
        "registered": registered,
        "missing": expected - registered,
        "unexpected": registered - expected,
    }

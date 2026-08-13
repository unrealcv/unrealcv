# Runtime MCP with [UnrealZoo](https://github.com/UnrealZoo)

Runtime MCP lets an agent inspect and control a running [UnrealZoo](https://github.com/UnrealZoo) environment
through compact structured tools. It is part of **UnrealCV Dev For [UnrealZoo](https://github.com/UnrealZoo)**:
the service is in active development, is tested in [UnrealZoo](https://github.com/UnrealZoo) first, and is not
included in the open-source UnrealCV plugin in this repository.

The public client, examples, and Codex skill live in
[`lizi-Margin/unrealcv-runtime-mcp`](https://github.com/lizi-Margin/unrealcv-runtime-mcp).
It is the public staging repository intended for transfer to the `unrealcv`
organization.
The Unreal Engine C++ server implementation is not currently open source.

## Prerequisites

1. Start a supported [UnrealZoo](https://github.com/UnrealZoo) environment with Runtime MCP enabled.
2. Confirm its log contains `Runtime MCP server listening on port 29998`.
3. Clone the public client repository and use Python 3.9 or newer. The example
   client has no third-party dependencies.

The service listens on `127.0.0.1:29998` by default and uses UnrealCV's framed
TCP transport. Keep it on a trusted network.

## Connect and discover tools

```powershell
git clone https://github.com/unrealcv/unrealcv-runtime-mcp.git
cd unrealcv-runtime-mcp
python .\examples\runtime_mcp_client.py ping
python .\examples\runtime_mcp_client.py tools
```

Always run `tools` after changing environments or builds. Its result is the
authoritative capability list for the connected runtime.

## Inspect a scene

Start with the compact scene overview:

```powershell
python .\examples\runtime_mcp_client.py call scene.overview --arguments '{"radius":2500,"max_actors":20}'
```

Select an actor name from that result before requesting more detail:

```powershell
python .\examples\runtime_mcp_client.py call scene.inspect_actor --arguments '{"actor":"ActorName"}'
```

Use `scene.capture_view` only when the structured overview and actor inspection
do not answer the task. Available capture arguments are described by `tools`.

## Use UnrealCV commands

The Runtime MCP `unrealcv` toolset exposes the existing command dispatcher:

- `unrealcv.list_cmd` lists commands registered by the running build.
- `unrealcv.describe_command` describes a command template.
- `unrealcv.exec` executes a raw UnrealCV command.

For example:

```powershell
python .\examples\runtime_mcp_client.py exec "vget /unrealcv/status"
python .\examples\runtime_mcp_client.py exec "vget /camera/0/location"
```

Do not infer command availability from a Python API method. Query the runtime
first. The open-source command contract is documented in
{doc}`Command System <../reference/commands>`; additional development commands
are marked {doc}`UnrealCV Dev For <reference/commands>` [UnrealZoo](https://github.com/UnrealZoo).

## Install the Codex skill

The public repository includes `skills/unrealcv-runtime-mcp`. Install that
folder in the Codex skills directory, then invoke `$unrealcv-runtime-mcp` to
follow the discovery-first inspection workflow. The skill prefers structured
scene tools, verifies capabilities before raw commands, and asks before broad or
destructive state changes.

## Protocol surface

The current service implements JSON-RPC 2.0 methods `initialize`, `ping`,
`tools/list`, and `tools/call` with MCP protocol version `2025-03-26`. Use the
public client instead of reimplementing the frame header in each integration.

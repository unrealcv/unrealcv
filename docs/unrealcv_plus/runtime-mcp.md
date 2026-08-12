# UnrealCV Runtime MCP

`UnrealCVMCP` is a runtime JSON-RPC MCP server listening on TCP port `29998`.
It uses UnrealCV's framed TCP transport and supports `initialize`, `ping`,
`tools/list`, and `tools/call`.

## Built-in toolsets

The `unrealcv` toolset exposes the existing command dispatcher:

- `unrealcv.list_cmd`: list registered UnrealCV commands.
- `unrealcv.describe_command`: describe one command URI template.
- `unrealcv.exec`: execute a raw UnrealCV command.

The `scene` toolset exposes agent-oriented runtime context:

- `scene.overview`: main camera pose/FOV and nearby actors, ordered by distance,
  with compact world-space bbox, annotation color, view angle, screen projection,
  and line-of-sight signals. It defaults to a 2500 cm radius and 20 actors;
  callers can override `radius` and `max_actors` (up to 100).
- `scene.inspect_actor`: transform, bounds, tags, ownership, and components for
  an actor returned by `scene.overview`.
- `scene.capture_view`: a `lit`, `normal`, or `object_mask` camera capture as MCP
  image content.

## Registering a toolset

Implement `IMCPToolset` from `MCPToolset.h`, then register a shared instance
during the owning runtime module's startup:

```cpp
FMCPToolRegistry::Get().RegisterToolset(MakeShared<FMyRuntimeToolset>());
```

Tool names must use the toolset name as their prefix, for example
`navigation.find_path` for a toolset whose `GetName()` returns `navigation`.
Unregister the toolset during module shutdown:

```cpp
FMCPToolRegistry::Get().UnregisterToolset(TEXT("navigation"));
```

The MCP server discovers definitions and dispatches calls only through the
registry, so adding a toolset does not require changes to the transport or
JSON-RPC implementation.

## Summary

This PR delivers a multi-pass hardening and modernization of the C++ server stack (networking, dispatch, command handlers, and tests) on top of `5.2`.

Primary goals addressed:
- correctness and crash prevention
- stronger null/error handling in command and transport paths
- tighter encapsulation and cleaner APIs
- lower per-request overhead in dispatcher/reply assembly
- improved automation-test coverage for server behavior

## What changed

### 1) Correctness and safety fixes
- Fixed input command bugs (`EnableInput`/`DisableInput`) and multiple null-deref risks in action/camera/alias/object handlers.
- Replaced unsafe hard assertions in runtime paths with graceful error returns where appropriate.
- Fixed stale-world usage (`GWorld`) and lifetime-sensitive captures (timer lambda now uses weak world).
- Ensured `WorldController::Tick` is a proper `override`.
- Added bounds checks for argument-indexed handlers.
- Fixed alias semantics so failing subcommands correctly propagate `Error` status.

### 2) Network transport hardening
- Added payload-size guards in TCP/UDS receive paths (OOM defense).
- Added retry backoff in send loops to avoid busy-spin behavior.
- Fixed echo services to handle partial sends correctly.
- Properly destroy sockets via socket subsystem cleanup.
- Made UDS transport mode flag thread-safe.
- Reset pending batch state on server errors.

### 3) Dispatcher and reply-path performance
- Reduced reply-buffer copies by adding `FExecStatus::AppendDataTo(...)` and using it in server reply assembly.
- Tightened dispatcher map lookups to avoid implicit insertions.
- Added route-verb prefilter in `Exec(...)` to skip unnecessary regex matching.
- Reserved capture-argument capacity in dispatch hot path.

### 4) Code quality and organization
- Modernized touched includes from legacy `Runtime/*/Public/...` style to UE5-style includes.
- Removed dead declarations (`PluginHandler` port methods).
- Reworked camera option parsing (projection/exposure/lit/reflection/illumination) into table-driven lookups.
- Reduced avoidable `FString` copies in command hot paths.
- Hardened `FActorController` against stale actor lifetime (`TWeakObjectPtr` + validity checks).

### 5) Tests
Added/extended automation tests to lock behavior:
- `AliasVRunCompatibility`
- `AliasErrorPropagation`
- Round-3 dispatcher/exec status tests guarded consistently with `WITH_AUTOMATION_WORKER`.

## Key files touched

- `Source/UnrealCV/Private/Server/UnixTcpServer.cpp`
- `Source/UnrealCV/Private/Server/TcpServer.cpp`
- `Source/UnrealCV/Private/Server/CommandDispatcher.cpp`
- `Source/UnrealCV/Public/Server/CommandDispatcher.h`
- `Source/UnrealCV/Private/Server/ExecStatus.cpp`
- `Source/UnrealCV/Public/Server/ExecStatus.h`
- `Source/UnrealCV/Private/Server/UnrealcvServer.cpp`
- `Source/UnrealCV/Private/Commands/ActionHandler.cpp`
- `Source/UnrealCV/Private/Commands/AliasHandler.cpp`
- `Source/UnrealCV/Private/Commands/CameraHandler.cpp`
- `Source/UnrealCV/Private/Commands/ObjectHandler.cpp`
- `Source/UnrealCV/Private/BPFunctionLib/VisionBPLib.cpp`
- `Source/UnrealCV/Public/Controller/ActorController.h`
- `Source/UnrealCV/Private/Controller/ActorController.cpp`
- `Source/UnrealCV/Private/Tests/AliasMultiCommandTest.cpp`

## Validation

- Local diagnostics (`get_errors`) are clean across `Source/UnrealCV`.
- New/updated tests were added for alias and dispatcher correctness.
- Full UE automation execution was not run in this environment (requires project runtime test execution context).

## Notes for reviewers

Suggested review order:
1. transport hardening (`TcpServer` / `UnixTcpServer`)
2. dispatch and status semantics (`CommandDispatcher` / `ExecStatus`)
3. handler correctness fixes (`Action` / `Alias` / `Camera` / `Object`)
4. behavior-locking tests (`AliasMultiCommandTest`)

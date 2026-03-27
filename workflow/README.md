# UnrealCV Debug Harness

A comprehensive debugging workflow system for UnrealCV plugin development with Claude Code integration.

## Overview

This harness provides a **closed-loop, feedback-driven workflow** for building, testing, and debugging UnrealCV:

```
┌─────────────────────────────────────────────────────────────┐
│  Full Workflow: build → launch → test → monitor logs        │
├─────────────────────────────────────────────────────────────┤
│  1. BUILD: Compile UE project with retry logic              │
│  2. LAUNCH: Start game and wait for UnrealCV server         │
│  3. TEST: Run connectivity and API tests                    │
│  4. LOGS: Real-time filtered log monitoring                 │
└─────────────────────────────────────────────────────────────┘
```

## Quick Start

```bash
# Full workflow (build + launch + test + logs)
cd workflow
python harness.py full

# Or individual phases
python harness.py build              # Build only
python harness.py launch             # Launch and monitor
python harness.py test               # Run tests
python harness.py logs               # Monitor log file
```

## Commands

### `full` - Complete Workflow

Runs all phases in sequence with automatic error handling:

```bash
python harness.py full
```

Options:
- `--clean` - Clean build before compiling
- `--skip-build` - Skip build phase
- `--skip-launch` - Skip game launch
- `--skip-test` - Skip test phase
- `--headless` - Run without rendering
- `--monitor-logs` - Continue monitoring after tests

### `build` - Compilation

Build the UE project with real-time log monitoring:

```bash
# Standard build
python harness.py build

# Clean build
python harness.py build --clean

# Custom config
python harness.py build --config myconfig.json
```

Features:
- Automatic retry on file lock/memory errors
- Real-time error/warning detection
- Build statistics and timing

### `launch` - Game Launch

Launch game and monitor logs:

```bash
# Standard launch
python harness.py launch

# Headless mode
python harness.py launch --headless

# Custom arguments
python harness.py launch -- -Windowed -ResX=1280 -ResY=720
```

### `test` - Test Execution

Run test suites:

```bash
# Basic tests only
python harness.py test

# Include pytest suite
python harness.py test --pytest

# Test existing server (skip launch)
python harness.py test --skip-launch
```

### `logs` - Log Monitoring

Monitor UE log file with filtering:

```bash
# Monitor with default filters
python harness.py logs

# Custom filter keywords
python harness.py logs --filter "UnrealCV,Error,Camera"

# Minimum log level
python harness.py logs --level Error

# Follow mode (default) or just show recent
python harness.py logs --no-follow
```

## Configuration

Create `config.json` based on the example:

```json
{
  "ue_path": "M:/UE_5.6/Engine",
  "project_path": "G:/HUAWEI_Project_UE56/HUAWEI_Project.uproject",
  "plugin_root": "G:/HUAWEI_Project_UE56/Plugins/unrealcv",
  "port": 9000,
  "log_filter_keywords": [
    "UnrealCV", "Error", "Warning", "Camera", "Sensor"
  ],
  "log_exclude_patterns": [
    "LogTemp", "LogInit"
  ],
  "build_timeout": 600,
  "max_build_retries": 2
}
```

`ue_path` is optional. Resolution order is:

1. `workflow/config.json`
2. `UE_PATH` environment variable
3. Auto-detect from the `.uproject` `EngineAssociation` using common install locations such as `M:/UE_5.6/Engine`

## Module Architecture

| Module | Purpose |
|--------|---------|
| `config.py` | Centralized configuration management |
| `builder.py` | UE build orchestration with retry logic |
| `test_runner.py` | Game launch, server detection, test execution |
| `log_monitor.py` | Real-time log parsing, filtering, streaming |
| `harness.py` | Main CLI controller |

## Integration with Claude Code

### As a Task Agent

The harness is designed for Claude Code task integration:

```python
# In a Claude Code task
from workflow.harness import DebugHarness

harness = DebugHarness()

# Run full workflow with progress updates
success = harness.run_full_workflow(args)

# Access results
build_result = harness._results.get('build')
test_result = harness._results.get('basic_tests')
```

### Status Callbacks

```python
def on_status_change(status, message):
    print(f"[{status.value}] {message}")

harness.builder.add_status_callback(on_status_change)
harness.test_runner.add_status_callback(on_status_change)
```

### Programmatic Control

```python
# Individual phase control
harness.phase_build(clean=True)
harness.phase_launch(headless=True)
harness.phase_test()

# Get filtered logs
errors = harness.test_runner.get_logs(level="Error", count=20)

# Save session logs
harness.test_runner.save_session_logs(Path("session.log"))
```

## Log Filtering

The log monitor supports multiple filtering strategies:

**Keyword Filtering:**
```python
log_monitor.filter.include_keywords = ["UnrealCV", "Camera", "Error"]
```

**Category Filtering:**
```python
log_monitor.filter.include_categories = ["LogUnrealCV", "LogCamera"]
```

**Level Filtering:**
```python
log_monitor.filter.include_levels = {"Error", "Warning", "Fatal"}
```

**Exclusion Patterns:**
```python
log_monitor.filter.exclude_patterns = ["LogTemp", "Verbose"]
```

## Error Recovery

The harness implements automatic error recovery:

1. **Build Phase**: Retries on file lock, access denied, timeout
2. **Launch Phase**: Waits for server with configurable timeout
3. **Test Phase**: Continues on individual test failures
4. **Signal Handling**: Clean shutdown on Ctrl+C

## Output Examples

### Build Output
```
====================================================================
                         PHASE 1: BUILD
====================================================================

[INFO] Executable: G:\HUAWEI_Project_UE56\Binaries\Win64\HUAWEI_Project.exe

[Build] Command: H:\UE_5.6\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.dll HUAWEI_Project Win64 Development -Project=G:\HUAWEI_Project_UE56\HUAWEI_Project.uproject ...

[Compiling] FusionCamCaptureActor.cpp
[Compiling] BaseCameraSensor.cpp
...
[Linking] HUAWEI_Project-Win64-Development.exe
[Succeeded] Build succeeded

[OK] Build completed in 17.3s
```

### Test Output
```
====================================================================
                         PHASE 3: TEST
====================================================================

[INFO] Running basic connectivity tests...

Test Results:
  [PASS] Connection           (0.52s)
  [PASS] Version              (0.01s)
  [PASS] Status               (0.01s)
  [PASS] Cameras              (0.02s)
  [PASS] Camera 0 Location    (0.01s)
  [PASS] Camera 0 Rotation    (0.01s)
  [PASS] Camera 0 FOV         (0.01s)
  [PASS] Objects              (0.05s)

Summary: 8/8 tests passed
[OK] All basic tests passed
```

### Log Monitor Output
```
====================================================================
                    LOG MONITOR (Press Ctrl+C to stop)
====================================================================

Filter keywords: UnrealCV, Error, Warning, Camera, Sensor
Log file: G:\HUAWEI_Project_UE56\Saved\Logs\HUAWEI_Project.log

[Log      ] [LogUnrealCV           ] UnrealCV server started on port 9000
[Log      ] [LogFusionCamSensor    ] Camera sensor initialized
[Warning  ] [LogCameraHandler      ] Camera 0 not found, using default
[Log      ] [LogUnrealCV           ] Client connected from 127.0.0.1
```

## Best Practices

1. **Use `--headless` for CI/CD** to avoid window focus issues
2. **Save logs with `--save-logs`** for post-mortem analysis
3. **Custom filters** for debugging specific subsystems
4. **Incremental builds** with `--skip-build` when only testing
5. **Monitor mode** with `logs` command for live debugging

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Build tool not found | Check `ue_path` in config |
| Server timeout | Increase `server_ready_timeout` |
| Port in use | Change `port` in config or kill existing process |
| Missing logs | Check `ue_log_path` configuration |
| Import errors | Ensure working directory is `workflow/` |

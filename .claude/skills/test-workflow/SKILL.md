---
name: test-workflow
description: Run UnrealCV closed-loop test workflow (build + launch + test)
model: sonnet
---

You are the UnrealCV Test Workflow Runner. Execute the complete build-test-debug pipeline using the workflow harness.

## Task

Run the UnrealCV debug harness to verify code changes work correctly:
1. **Build** - Compile UE project with UnrealCV plugin
2. **Launch** - Start game and wait for UnrealCV server
3. **Test** - Run connectivity and API tests
4. **Report** - Show results and any errors

## Execution Modes

### Default: Full Workflow
Run complete pipeline (build + launch + test):
```bash
cd workflow && python harness.py full
```

### Build Only
Just compile without testing:
```bash
cd workflow && python harness.py build
```

## Configuration

The harness uses default settings from `config.py`. If you need custom paths, create `workflow/config.json`:

```json
{
  "ue_path": "H:/UE_5.6/Engine",
  "project_path": "G:/HUAWEI_Project_UE56/HUAWEI_Project.uproject",
  "plugin_root": "G:/HUAWEI_Project_UE56/Plugins/unrealcv",
  "port": 9000,
  "log_filter_keywords": ["UnrealCV", "Error", "Warning", "Camera", "Sensor"],
  "post_launch_delay": 5.0  // Seconds to wait after server ready before tests
}
```

**Important configs:**
- `post_launch_delay`: Time to wait after server ready before running tests (default: 3.0s). Increase this if shaders need time to compile.
- `server_ready_timeout`: Max time to wait for server to start (default: 60s)

## Windows Paths Caution
Use `/` rather than `\\`, `\` for paths, e.g. `G:/HUAWEI_Project_UE56` rather than `G:\\HUAWEI_Project_UE56`, `G:\HUAWEI_Project_UE56`. 

### Bad Use Cases
 Bash(cd G:\HUAWEI_Project_UE56\Plugins\unrealcv\workflow && python harness.py full)
  ⎿  Error: Exit code 1
     /usr/bin/bash: line 1: cd: G:HUAWEI_Project_UE56Pluginsunrealcvworkflow: No such file or directory
### Good Use Cases
● Bash(python workflow/harness.py full)

## Execution Steps

### Step 1: Verify Environment

1. Check workflow directory exists: `workflow/`
2. Verify default paths in `config.py` match your environment (or create `config.json` to override)
3. Verify UE path and project path are valid

### Step 2: Run Build

Execute build phase:
```bash
cd workflow && python harness.py build
```

Monitor for:
- Compilation errors in UnrealCV plugin files
- Link errors
- Warnings that might indicate issues

**If build fails**:
- Check for syntax errors in modified files
- Verify all includes are correct
- Check for missing dependencies

## Build Logs

Build logs are automatically saved to `workflow/debug_logs/` directory:

```
workflow/debug_logs/build_<TargetName>_<YYYYMMDD_HHMMSS>.log
```

**Accessing build logs from the skill:**

After running build, the log file path is stored in `BuildResult.log_path`:

```python
from workflow.builder import UEBuilder

builder = UEBuilder()
result = builder.build()

# Log file path is available immediately after build
if result.log_path:
    print(f"Build log saved to: {result.log_path}")
    # Read the full build log
    with open(result.log_path, 'r') as f:
        build_log_content = f.read()
```

**Finding the latest build log:**

```python
from pathlib import Path
import glob

log_dir = Path("workflow/debug_logs")
if log_dir.exists():
    log_files = list(log_dir.glob("build_*.log"))
    if log_files:
        latest_log = max(log_files, key=lambda p: p.stat().st_mtime)
        print(f"Latest build log: {latest_log}")
```

**In the harness output:**
The build phase displays the log path:
```
[Build] Log file: workflow/debug_logs/build_HUAWEI_Project_20250311_143052.log
[OK] Build completed in 45.2s
Build log: workflow/debug_logs/build_HUAWEI_Project_20250311_143052.log
```

### Step 3: Run Tests

Execute full test suite:
```bash
cd workflow && python harness.py full
```

Or with headless mode (no window):
```bash
cd workflow && python harness.py full --headless
```

### Step 4: Analyze Results

**Success indicators**:
```
[OK] Build completed in XXXs
[OK] Server ready
[OK] All basic tests passed
Test Results:
  [PASS] Connection            (0.01s)
  [PASS] Version               (0.02s)
  [PASS] Status                (0.01s)
  [PASS] Cameras               (0.01s)
  [PASS] Camera 0 Location     (0.01s)
  [PASS] Camera 0 Rotation     (0.01s)
  [PASS] Camera 0 FOV          (0.01s)
  [PASS] Objects               (0.02s)
  [PASS] Capture Lit           (0.15s)  # Image capture tests
  [PASS] Capture Depth         (0.12s)
  [PASS] Capture Normal        (0.14s)
  [PASS] Capture ObjectMask    (0.13s)

Summary: 12/12 passed
```

**Failure indicators**:
- Build errors (compilation/linking)
- Server timeout (game didn't start)
- Test failures (API not working)

## Test Coverage

The workflow runs these tests:

### Basic Connectivity Tests
1. **Connection** - TCP connection to UnrealCV server
2. **Version** - Get plugin version
3. **Status** - Get server status
4. **Cameras** - List available cameras
5. **Camera 0 Location** - Get camera position
6. **Camera 0 Rotation** - Get camera rotation
7. **Camera 0 FOV** - Get camera field of view
8. **Objects** - List scene objects

### Image Capture Tests (with `post_launch_delay` wait)
9. **Capture Lit** - `vget /camera/0/lit` - RGB image capture
10. **Capture Depth** - `vget /camera/0/depth` - Depth map capture
11. **Capture Normal** - `vget /camera/0/normal` - Normal map capture
12. **Capture ObjectMask** - `vget /camera/0/object_mask` - Segmentation mask
13. **Capture OpticalFlow** - `vget /camera/0/optical_flow` - Optical flow (if available)

## Common Issues & Solutions

### Build Issues
| Error | Solution |
|-------|----------|
| Build tool not found | Check `ue_path` in config.json |
| Compilation error | Fix syntax in modified .cpp/.h files |
| Link error | Check all function declarations have definitions |

### Launch Issues
| Error | Solution |
|-------|----------|
| Server timeout | Increase `server_ready_timeout` in config |
| Port in use | Kill existing process or change port |
| Game crashes | Check UE logs for assertion failures |

### Test Issues
| Error | Solution |
|-------|----------|
| Connection refused | Server not started, check launch phase |
| Command timeout | Command handler not registered or crashed |
| Wrong response | Command implementation bug |

## Log Monitoring

After tests pass, monitor logs for warnings:
```bash
cd workflow && python harness.py logs --filter "UnrealCV,Error,Warning"
```

## Report Format

After execution, provide summary:

```
UnrealCV Test Workflow Report
==============================

Build Phase:
  Status: ✓ SUCCESS / ✗ FAILED
  Duration: XXXs
  Warnings: N (if any)
  Log File: workflow/debug_logs/build_xxx.log

Launch Phase:
  Status: ✓ SUCCESS / ✗ FAILED
  Server startup: XXs
  Post-launch delay: X.Xs (for shader compilation)

Test Phase:
  Status: ✓ PASSED / ✗ FAILED
  Passed: N/12 (basic + capture tests)
  Failed tests: (list if any)
  Capture tests: ✓ Lit, Depth, Normal, ObjectMask (OpticalFlow if available)

Overall: ✓ ALL TESTS PASSED / ✗ WORKFLOW FAILED

Recommendations:
- If build failed: Fix compilation errors, check build log
- If tests failed: Check command implementations, verify image capture sensors
- If capture tests failed: May need longer post_launch_delay for shader compilation
- If all passed: Code is ready for commit
```

## Usage Examples

**After making code changes**:
```
User: /test-workflow
```
**Build only**:
```
User: /test-workflow --build-only
```

**With headless mode**:
```
User: /test-workflow --headless
```

## Exit Codes

- 0: All tests passed
- 1: Build or test failed

## Notes

- First build may take 3-5 minutes
- Subsequent builds are faster (incremental)
- Headless mode is useful for CI/CD
- Game window may steal focus during launch

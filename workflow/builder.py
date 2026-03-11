"""
UnrealCV Debug Harness - Build Module
Handles UE project and plugin compilation with retry logic.
"""
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional, List, Callable
from dataclasses import dataclass
from enum import Enum

from config import get_config
from log_monitor import LogMonitor, LogEntry, ConsoleLogPrinter


class BuildStatus(Enum):
    PENDING = "pending"
    COMPILING = "compiling"
    SUCCESS = "success"
    FAILED = "failed"
    RETRYING = "retrying"
    CANCELLED = "cancelled"


@dataclass
class BuildResult:
    status: BuildStatus
    return_code: int
    duration: float
    log_path: Optional[Path] = None
    errors: List[str] = None
    warnings: List[str] = None

    def __post_init__(self):
        if self.errors is None:
            self.errors = []
        if self.warnings is None:
            self.warnings = []


class UEBuilder:
    """Unreal Engine project builder with monitoring"""

    def __init__(self):
        self.config = get_config()
        self.status = BuildStatus.PENDING
        self._callbacks: List[Callable[[BuildStatus, str], None]] = []
        self._current_proc: Optional[subprocess.Popen] = None
        self._cancelled = False

    def add_status_callback(self, callback: Callable[[BuildStatus, str], None]):
        """Add status change callback"""
        self._callbacks.append(callback)

    def _notify_status(self, status: BuildStatus, message: str = ""):
        """Notify status change"""
        self.status = status
        for callback in self._callbacks:
            try:
                callback(status, message)
            except Exception as e:
                print(f"Callback error: {e}")

    def cancel(self):
        """Cancel current build"""
        self._cancelled = True
        if self._current_proc:
            self._current_proc.terminate()

    def build(self, target: Optional[str] = None,
              configuration: str = "Development",
              platform: str = "Win64",
              clean: bool = False) -> BuildResult:
        """
        Build the UE project with monitoring and retry logic.

        Args:
            target: Build target (defaults to project name)
            configuration: Build configuration (Development, Shipping, DebugGame)
            platform: Target platform (Win64, Linux, Mac)
            clean: Whether to clean before building
        """
        config = self.config
        build_tool = config.build_tool_path

        if not build_tool:
            return BuildResult(
                status=BuildStatus.FAILED,
                return_code=-1,
                duration=0,
                errors=["Build tool not found"]
            )

        if target is None:
            target = config.project_path.stem

        # Create log file path
        timestamp = time.strftime("%Y%m%d_%H%M%S")
        log_dir = Path(config.log_output_dir)
        log_dir.mkdir(parents=True, exist_ok=True)
        log_path = log_dir / f"build_{target}_{timestamp}.log"

        # Track attempts
        attempt = 0
        max_retries = config.max_build_retries

        while attempt <= max_retries:
            if self._cancelled:
                return BuildResult(
                    status=BuildStatus.CANCELLED,
                    return_code=-1,
                    duration=0
                )

            attempt += 1
            if attempt > 1:
                self._notify_status(BuildStatus.RETRYING, f"Retry attempt {attempt}/{max_retries + 1}")
                time.sleep(2)

            self._notify_status(BuildStatus.COMPILING, f"Building {target} ({configuration})")

            # Build command
            cmd = [
                str(build_tool),
                target,
                platform,
                configuration,
                f"-Project={config.project_path}",
                "-WaitMutex",
                "-FromMsBuild",
                f"-architecture=x64",
            ]

            if clean:
                cmd.insert(1, "-Clean")

            print(f"\n[Build] Command: {' '.join(cmd)}\n")
            print(f"[Build] Log file: {log_path}\n")

            start_time = time.time()

            # Setup log monitor
            log_monitor = LogMonitor(buffer_size=500)
            printer = ConsoleLogPrinter(show_category=False)

            # Configure filter for build logs
            log_monitor.filter.include_keywords = [
                "error", "warning", "failed", "succeeded",
                "Compiling", "Linking", "ERROR", "WARN"
            ]
            log_monitor.filter.include_levels = {"Error", "Warning"}
            log_monitor.add_callback(printer)

            errors = []
            warnings = []
            full_log_lines = []

            def collect_logs(entry: LogEntry):
                if entry.level == "Error":
                    errors.append(entry.message)
                elif entry.level == "Warning":
                    warnings.append(entry.message)

            log_monitor.add_callback(collect_logs)

            # Open log file for writing
            with open(log_path, 'w', encoding='utf-8') as log_file:
                try:
                    self._current_proc = subprocess.Popen(
                        cmd,
                        cwd=str(config.ue_path.parent),
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                        text=True,
                        bufsize=1,
                        encoding='utf-8',
                        errors='ignore'
                    )

                    # Monitor output
                    for line in self._current_proc.stdout:
                        # Write to log file
                        log_file.write(line)
                        log_file.flush()
                        full_log_lines.append(line)
                        log_monitor.feed_line(line)
                        if self._cancelled:
                            self._current_proc.terminate()
                            break

                    self._current_proc.wait()
                    duration = time.time() - start_time

                    if self._cancelled:
                        return BuildResult(
                            status=BuildStatus.CANCELLED,
                            return_code=-1,
                            duration=duration,
                            log_path=log_path
                        )

                    if self._current_proc.returncode == 0:
                        self._notify_status(BuildStatus.SUCCESS, f"Build completed in {duration:.1f}s")
                        return BuildResult(
                            status=BuildStatus.SUCCESS,
                            return_code=0,
                            duration=duration,
                            log_path=log_path,
                            warnings=warnings
                        )
                    else:
                        # Check for retryable errors
                        retryable = self._is_retryable_error(errors)
                        if retryable and attempt <= max_retries:
                            print(f"\n[Build] Detected retryable error, will retry...")
                            continue

                        self._notify_status(BuildStatus.FAILED, f"Build failed in {duration:.1f}s")
                        return BuildResult(
                            status=BuildStatus.FAILED,
                            return_code=self._current_proc.returncode,
                            duration=duration,
                            log_path=log_path,
                            errors=errors,
                            warnings=warnings
                        )

                except subprocess.TimeoutExpired:
                    duration = time.time() - start_time
                    self._current_proc.kill()
                    self._notify_status(BuildStatus.FAILED, "Build timed out")
                    return BuildResult(
                        status=BuildStatus.FAILED,
                        return_code=-1,
                        duration=duration,
                        log_path=log_path,
                        errors=["Build timed out"]
                    )
                except Exception as e:
                    duration = time.time() - start_time
                    self._notify_status(BuildStatus.FAILED, f"Build error: {e}")
                    return BuildResult(
                        status=BuildStatus.FAILED,
                        return_code=-1,
                        duration=duration,
                        log_path=log_path,
                        errors=[str(e)]
                    )

        # Exhausted retries
        return BuildResult(
            status=BuildStatus.FAILED,
            return_code=-1,
            duration=0,
            log_path=log_path,
            errors=["Max retries exceeded"] + errors,
            warnings=warnings
        )

    def _is_retryable_error(self, errors: List[str]) -> bool:
        """Check if errors are retryable (e.g., file locked, memory)"""
        retryable_patterns = [
            "mutex", "locked", "access denied", "file in use",
            "out of memory", "temporarily", "timeout"
        ]
        for error in errors:
            error_lower = error.lower()
            for pattern in retryable_patterns:
                if pattern in error_lower:
                    return True
        return False

    def build_plugin_only(self) -> BuildResult:
        """Build only the UnrealCV plugin"""
        config = get_config()
        build_tool = config.build_tool_path

        if not build_tool:
            return BuildResult(
                status=BuildStatus.FAILED,
                return_code=-1,
                duration=0,
                errors=["Build tool not found"]
            )

        # Build the plugin using RunUAT
        cmd = [
            str(config.ue_path / "Build/BatchFiles/RunUAT.bat"),
            "BuildPlugin",
            f"-Plugin={config.plugin_root / 'UnrealCV.uplugin'}",
            f"-Package={config.plugin_root / 'Build/Plugin'}",
            "-TargetPlatforms=Win64",
        ]

        print(f"\n[Build Plugin] Command: {' '.join(cmd)}\n")

        start_time = time.time()

        try:
            result = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=config.build_timeout
            )

            duration = time.time() - start_time

            if result.returncode == 0:
                return BuildResult(
                    status=BuildStatus.SUCCESS,
                    return_code=0,
                    duration=duration
                )
            else:
                return BuildResult(
                    status=BuildStatus.FAILED,
                    return_code=result.returncode,
                    duration=duration,
                    errors=[result.stderr[-2000:]] if result.stderr else ["Unknown error"]
                )
        except Exception as e:
            return BuildResult(
                status=BuildStatus.FAILED,
                return_code=-1,
                duration=time.time() - start_time,
                errors=[str(e)]
            )

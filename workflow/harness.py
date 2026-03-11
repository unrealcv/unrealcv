#!/usr/bin/env python3
"""
UnrealCV Debug Harness - Main Controller
Orchestrates build, launch, test, and log monitoring workflows.

Usage:
    python harness.py [command] [options]

Commands:
    build           Build the UE project
    launch          Launch game and monitor logs
    test            Run test suite (launch + test + logs)
    full            Full workflow (build + launch + test + logs)
    logs            Monitor UE log file only
    status          Show current status

Options:
    --config FILE       Use custom config file
    --clean             Clean build
    --filter KEYWORDS   Comma-separated log filter keywords
    --level LEVEL       Minimum log level (Fatal/Error/Warning/Log)
    --headless          Run game in headless mode
    --timeout SECS      Override timeout
    --save-logs FILE    Save logs to file

Examples:
    # Full debug workflow
    python harness.py full

    # Build only with custom config
    python harness.py build --config myconfig.json --clean

    # Launch and monitor with custom filters
    python harness.py launch --filter "UnrealCV,Error,Camera" --level Error

    # Run tests on existing server
    python harness.py test --skip-launch

    # Monitor log file
    python harness.py logs --filter "Sensor,Recording"
"""
import os
import sys
import time
import json
import signal
import argparse
from pathlib import Path
from typing import Optional, List
from datetime import datetime

# Ensure imports work
WORKFLOW_DIR = Path(__file__).resolve().parent
PLUGIN_ROOT = WORKFLOW_DIR.parent
sys.path.insert(0, str(WORKFLOW_DIR))

from config import get_config, set_config, UEConfig
from builder import UEBuilder, BuildStatus
from test_runner import UETestRunner, TestStatus
from log_monitor import LogMonitor, LogEntry, ConsoleLogPrinter


class Colors:
    """No colors - for agent parsing"""
    @classmethod
    def enable_windows(cls):
        pass


class DebugHarness:
    """Main debug harness controller"""

    def __init__(self):
        self.config = get_config()
        self.builder = UEBuilder()
        self.test_runner = UETestRunner()
        self._running = False
        self._current_phase = "idle"
        self._results = {}

        # Setup signal handlers
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)

        Colors.enable_windows()

    def _signal_handler(self, signum, frame):
        """Handle interrupt signals"""
        print("INTERRUPTED")
        self.shutdown()
        sys.exit(1)

    def shutdown(self):
        """Clean shutdown"""
        self._running = False
        self.test_runner.cancel()
        self.builder.cancel()
        self.test_runner.stop_game()

    def _print_header(self, text: str):
        """Print phase header"""
        print(f"=== {text} ===")

    def _print_status(self, text: str, status: str = "info"):
        """Print status message"""
        print(f"[{status.upper()}] {text}")

    def phase_build(self, clean: bool = False) -> bool:
        """Build phase"""
        self._current_phase = "build"
        self._print_header("PHASE 1: BUILD")

        result = self.builder.build(clean=clean)
        self._results['build'] = result

        if result.status == BuildStatus.SUCCESS:
            self._print_status(f"Build completed in {result.duration:.1f}s", "success")
            if result.log_path:
                self._print_status(f"Build log: {result.log_path}")
            if result.warnings:
                self._print_status(f"  Warnings: {len(result.warnings)}", "warning")
            return True
        else:
            self._print_status(f"Build failed: {result.status.value}", "error")
            if result.log_path:
                self._print_status(f"Build log: {result.log_path}")
            if result.errors:
                print(f"\n{Colors.FAIL}Errors:{Colors.ENDC}")
                for err in result.errors[:10]:
                    print(f"  - {err[:200]}")
            return False

    def phase_launch(self, headless: bool = False, extra_args: Optional[List[str]] = None) -> bool:
        """Launch phase"""
        self._current_phase = "launch"
        self._print_header("PHASE 2: LAUNCH")

        self._print_status(f"Executable: {self.config.exe_path}")

        launch_args = []
        if headless:
            launch_args.append("-RenderOffScreen")
        if extra_args:
            launch_args.extend(extra_args)

        if not self.test_runner.launch_game(launch_args):
            self._print_status("Failed to launch game", "error")
            return False

        self._print_status("Game launched, waiting for server...")

        if not self.test_runner.wait_for_server():
            self._print_status("Server failed to start", "error")
            return False

        self._print_status("Server ready", "success")
        return True

    def phase_test(self, skip_basic: bool = False, pytest_suite: bool = False) -> bool:
        """Test phase"""
        self._current_phase = "test"
        self._print_header("PHASE 3: TEST")

        success = True

        if not skip_basic:
            self._print_status("Running basic connectivity tests...")
            result = self.test_runner.run_basic_tests()
            self._results['basic_tests'] = result

            for test in result.results:
                status_text = "PASS" if test.status == TestStatus.PASSED else "FAIL"
                print(f"{status_text}|{test.name}|{test.duration:.2f}s")
                if test.status == TestStatus.FAILED and test.message:
                    print(f"ERROR|{test.name}|{test.message[:100]}")

            print(f"SUMMARY|{result.passed}/{result.total_tests} passed")

            if result.overall_status == TestStatus.PASSED:
                self._print_status("All basic tests passed", "success")
            else:
                self._print_status(f"{result.failed} tests failed", "error")
                success = False

        if pytest_suite:
            self._print_status("Running pytest suite...")
            result = self.test_runner.run_pytest_suite()
            self._results['pytest'] = result

            print(f"PYTEST|Passed:{result.passed}|Failed:{result.failed}|Duration:{result.duration:.1f}s")

            if result.overall_status != TestStatus.PASSED:
                success = False

        return success

    def phase_logs(self, follow: bool = True, duration: Optional[int] = None):
        """Log monitoring phase"""
        self._current_phase = "logs"

        if not follow:
            # Just show recent logs
            logs = self.test_runner.get_logs(count=50)
            for log in logs:
                print(log)
            return

        # Interactive log monitoring
        self._print_header("LOG MONITOR")

        start_time = time.time()

        try:
            while self._running:
                if duration and (time.time() - start_time) > duration:
                    break
                time.sleep(0.5)

        except KeyboardInterrupt:
            print("STOPPED")

    def run_full_workflow(self, args) -> bool:
        """Run complete workflow"""
        self._running = True
        overall_start = time.time()

        try:
            # Phase 1: Build
            if not args.skip_build:
                if not self.phase_build(clean=args.clean):
                    return False
            else:
                self._print_status("Skipping build phase", "warning")

            # Phase 2: Launch
            if not args.skip_launch:
                if not self.phase_launch(headless=args.headless):
                    self.shutdown()
                    return False
            else:
                self._print_status("Skipping launch phase", "warning")

            # Phase 3: Test
            if not args.skip_test:
                self.phase_test(skip_basic=args.skip_basic, pytest_suite=args.pytest)
            else:
                self._print_status("Skipping test phase", "warning")

            # Phase 4: Log monitoring
            if args.monitor_logs:
                self.phase_logs(follow=True, duration=args.monitor_duration)

            # Cleanup
            if not args.no_cleanup:
                time.sleep(5)
                self.test_runner.stop_game()

            total_duration = time.time() - overall_start
            self._print_header(f"WORKFLOW COMPLETE|{total_duration:.1f}s")

            return True

        except Exception as e:
            self._print_status(f"Workflow error: {e}", "error")
            self.shutdown()
            return False

    def run_build_only(self, args) -> bool:
        """Run build command only"""
        return self.phase_build(clean=args.clean)

    def run_launch_only(self, args) -> bool:
        """Run launch command only"""
        self._running = True

        if not self.phase_launch(headless=args.headless):
            return False

        # Keep running and monitor logs
        try:
            self.phase_logs(follow=True)
        finally:
            self.test_runner.stop_game()

        return True

    def run_test_only(self, args) -> bool:
        """Run test command only"""
        if not args.skip_launch:
            if not self.phase_launch(headless=args.headless):
                return False

        return self.phase_test(skip_basic=args.skip_basic, pytest_suite=args.pytest)

    def run_logs_only(self, args) -> bool:
        """Run log monitoring only"""
        # Setup log monitor for file
        log_monitor = LogMonitor(buffer_size=self.config.log_buffer_size)
        printer = ConsoleLogPrinter(show_category=True)

        # Configure filter
        if args.filter:
            log_monitor.filter.include_keywords = args.filter.split(',')
        else:
            log_monitor.filter.include_keywords = self.config.log_filter_keywords

        if args.level:
            log_monitor.filter.include_levels = {args.level}

        log_monitor.add_callback(printer)

        # Start monitoring file
        log_file = self.config.ue_log_path
        log_monitor.start_monitoring_file(log_file, follow=True)

        print(f"Monitoring: {log_file}")
        print("Press Ctrl+C to stop...\n")

        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nStopped.")
        finally:
            log_monitor.stop()

        return True


def main():
    parser = argparse.ArgumentParser(
        description="UnrealCV Debug Harness",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    parser.add_argument(
        "command",
        choices=["full", "build", "launch", "test", "logs", "status"],
        help="Command to execute"
    )

    # Config options
    parser.add_argument("--config", type=Path, help="Custom config file")
    parser.add_argument("--clean", action="store_true", help="Clean build")

    # Phase skips
    parser.add_argument("--skip-build", action="store_true", help="Skip build phase")
    parser.add_argument("--skip-launch", action="store_true", help="Skip launch phase")
    parser.add_argument("--skip-test", action="store_true", help="Skip test phase")
    parser.add_argument("--skip-basic", action="store_true", help="Skip basic tests")

    # Launch options
    parser.add_argument("--headless", action="store_true", help="Run headless")
    parser.add_argument("--no-cleanup", action="store_true", help="Don't stop game after tests")

    # Test options
    parser.add_argument("--pytest", action="store_true", help="Run pytest suite")

    # Log options
    parser.add_argument("--monitor-logs", action="store_true", help="Monitor logs after tests")
    parser.add_argument("--monitor-duration", type=int, help="Log monitoring duration in seconds")
    parser.add_argument("--filter", help="Comma-separated log filter keywords")
    parser.add_argument("--level", choices=["Fatal", "Error", "Warning", "Log"], help="Minimum log level")
    parser.add_argument("--save-logs", type=Path, help="Save logs to file")

    args = parser.parse_args()

    # Load custom config if specified
    if args.config:
        config = UEConfig.from_file(args.config)
        set_config(config)

    # Create harness and execute
    harness = DebugHarness()

    if args.command == "full":
        success = harness.run_full_workflow(args)
    elif args.command == "build":
        success = harness.run_build_only(args)
    elif args.command == "launch":
        success = harness.run_launch_only(args)
    elif args.command == "test":
        success = harness.run_test_only(args)
    elif args.command == "logs":
        success = harness.run_logs_only(args)
    elif args.command == "status":
        print(f"Status: {harness._current_phase}")
        print(f"Config: {harness.config.project_path}")
        success = True
    else:
        parser.print_help()
        success = False

    sys.stdout.flush()
    os._exit(0 if success else 1)


if __name__ == "__main__":
    main()

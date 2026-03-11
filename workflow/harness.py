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

    def phase_test(self) -> bool:
        """Test phase"""
        self._current_phase = "test"
        self._print_header("PHASE 3: TEST")

        success = True


        self._print_status("Running basic connectivity tests...")
        result = self.test_runner.run_basic_tests()
        self._results['basic_tests'] = result

        for test in result.results:
            status_text = "PASS" if test.status == TestStatus.PASSED else "FAIL"
            print(f"{status_text}|{test.name}|{test.duration:.2f}s")
            if test.status == TestStatus.FAILED and test.message:
                print(f"ERROR|{test.name}|{test.message}")

        print(f"SUMMARY|{result.passed}/{result.total_tests} passed")

        if result.overall_status == TestStatus.PASSED:
            self._print_status("All basic tests passed", "success")
        else:
            self._print_status(f"{result.failed} tests failed", "error")
            success = False

        return success

    def run_full_workflow(self, args) -> bool:
        """Run complete workflow"""
        self._running = True
        overall_start = time.time()

        try:
            if not self.phase_build(clean=args.clean):
                return False

            if not self.phase_launch(headless=args.headless):
                self.shutdown()
                return False
            
            self.phase_test()
            time.sleep(2.0)
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

def main():
    parser = argparse.ArgumentParser(
        description="UnrealCV Debug Harness",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )

    parser.add_argument(
        "command",
        choices=["full", "build", "status"],
        help="Command to execute"
    )

    # Config options
    parser.add_argument("--config", type=Path, help="Custom config file")
    parser.add_argument("--clean", action="store_true", help="Clean build")

    # Launch options
    parser.add_argument("--headless", action="store_true", help="Run headless")
    parser.add_argument("--no-cleanup", action="store_true", help="Don't stop game after tests")

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

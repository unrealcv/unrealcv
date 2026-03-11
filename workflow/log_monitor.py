"""
UnrealCV Debug Harness - Log Monitor
Real-time UE log monitoring with filtering and streaming capabilities.
"""
import re
import time
import queue
import threading
from pathlib import Path
from typing import List, Optional, Callable, Set
from dataclasses import dataclass, field
from datetime import datetime
from collections import deque
import subprocess


@dataclass
class LogEntry:
    """Single log entry"""
    timestamp: datetime
    level: str  # Log, Warning, Error, Fatal, Display
    category: str  # LogCategory
    message: str
    raw_line: str

    @classmethod
    def parse(cls, line: str) -> Optional["LogEntry"]:
        """Parse a UE log line"""
        # Try standard UE log format first: [2024.01.15-12.34.56:789][  0]LogCategory: Message
        pattern = r'\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d+)\]\[\s*\d+\]([^:]+):\s*(.*)'
        match = re.match(pattern, line.strip())

        if match:
            time_str, category, message = match.groups()
            try:
                timestamp = datetime.strptime(time_str, "%Y.%m.%d-%H.%M.%S:%f")
            except ValueError:
                timestamp = datetime.now()

            # Detect log level
            level = "Log"
            for lvl in ["Fatal", "Error", "Warning", "Display", "Verbose", "VeryVerbose"]:
                if message.startswith(f"{lvl}: "):
                    level = lvl
                    message = message[len(lvl)+2:]
                    break

            return cls(timestamp, level, category, message, line.strip())

        # Try pipe format: LogCategory: Level: Message or LogCategory: Message
        # Format examples:
        # LogUnrealCV: Error: Failed to load material...
        # LogUnrealCV: Display: Loading configuration...
        pipe_pattern = r'^([A-Za-z0-9_]+):\s*(Fatal|Error|Warning|Display|Verbose|VeryVerbose)?:?\s*(.*)'
        pipe_match = re.match(pipe_pattern, line.strip())

        if pipe_match:
            category, level, message = pipe_match.groups()
            if level is None:
                level = "Log"
            return cls(
                timestamp=datetime.now(),
                level=level,
                category=category,
                message=message,
                raw_line=line.strip()
            )

        return None

    def __str__(self) -> str:
        return f"[{self.level}] [{self.category}] {self.message}"


class LogFilter:
    """Configurable log filter"""

    def __init__(self):
        self.include_keywords: List[str] = []
        self.exclude_patterns: List[str] = []
        self.include_levels: Set[str] = set()
        self.include_categories: List[str] = []

    def matches(self, entry: LogEntry) -> bool:
        """Check if log entry matches filter criteria"""
        # Level filter
        if self.include_levels and entry.level not in self.include_levels:
            return False

        # Category filter
        if self.include_categories:
            if not any(cat.lower() in entry.category.lower() for cat in self.include_categories):
                return False

        # Exclude patterns
        for pattern in self.exclude_patterns:
            if pattern.lower() in entry.message.lower() or pattern.lower() in entry.category.lower():
                return False

        # Include keywords (if specified, at least one must match)
        if self.include_keywords:
            text = f"{entry.category} {entry.message}".lower()
            if not any(kw.lower() in text for kw in self.include_keywords):
                return False

        return True


class LogMonitor:
    """
    Real-time UE log monitor with multiple input sources:
    - Process stdout/stderr
    - Log file tailing
    - Direct stream input
    """

    def __init__(self, buffer_size: int = 1000):
        self.buffer: deque = deque(maxlen=buffer_size)
        self.filter = LogFilter()
        self.callbacks: List[Callable[[LogEntry], None]] = []
        self.running = False
        self._thread: Optional[threading.Thread] = None
        self._queue: queue.Queue = queue.Queue()
        self._stats = {
            'total_lines': 0,
            'filtered_lines': 0,
            'error_count': 0,
            'warning_count': 0,
        }

    def add_callback(self, callback: Callable[[LogEntry], None]):
        """Add a callback for filtered log entries"""
        self.callbacks.append(callback)

    def remove_callback(self, callback: Callable[[LogEntry], None]):
        """Remove a callback"""
        if callback in self.callbacks:
            self.callbacks.remove(callback)

    def _notify(self, entry: LogEntry):
        """Notify all callbacks"""
        for callback in self.callbacks:
            try:
                callback(entry)
            except Exception as e:
                print(f"Callback error: {e}")

    def feed_line(self, line: str):
        """Feed a single log line"""
        self._stats['total_lines'] += 1

        entry = LogEntry.parse(line)
        if entry is None:
            # Try to create a basic entry for unparsable lines
            entry = LogEntry(
                timestamp=datetime.now(),
                level="Unknown",
                category="Raw",
                message=line.strip(),
                raw_line=line.strip()
            )

        # Update stats
        if entry.level == "Error":
            self._stats['error_count'] += 1
        elif entry.level == "Warning":
            self._stats['warning_count'] += 1

        self.buffer.append(entry)

        if self.filter.matches(entry):
            self._stats['filtered_lines'] += 1
            self._notify(entry)

    def feed_stream(self, stream, stop_event: Optional[threading.Event] = None):
        """Feed from a stream (file or pipe)"""
        while (stop_event is None or not stop_event.is_set()):
            try:
                line = stream.readline()
                if not line:
                    time.sleep(0.1)
                    continue
                self.feed_line(line)
            except Exception as e:
                print(f"Stream read error: {e}")
                break

    def start_monitoring_process(self, proc: subprocess.Popen):
        """Start monitoring a subprocess output"""
        self.running = True
        self._stop_event = threading.Event()

        def monitor():
            if proc.stdout:
                self.feed_stream(proc.stdout, self._stop_event)

        self._thread = threading.Thread(target=monitor, daemon=True)
        self._thread.start()

    def start_monitoring_file(self, log_path: Path, follow: bool = True):
        """Start monitoring a log file (like tail -f)"""
        self.running = True
        self._stop_event = threading.Event()

        def monitor():
            if not log_path.exists():
                print(f"Log file not found: {log_path}")
                return

            with open(log_path, 'r', encoding='utf-8', errors='ignore') as f:
                # Seek to end if following
                if follow:
                    f.seek(0, 2)

                while not self._stop_event.is_set():
                    line = f.readline()
                    if not line:
                        if not follow:
                            break
                        time.sleep(0.1)
                        continue
                    self.feed_line(line)

        self._thread = threading.Thread(target=monitor, daemon=True)
        self._thread.start()

    def stop(self):
        """Stop monitoring"""
        self.running = False
        if hasattr(self, '_stop_event'):
            self._stop_event.set()
        if self._thread:
            self._thread.join(timeout=2)

    def get_recent(self, count: int = 50, level: Optional[str] = None) -> List[LogEntry]:
        """Get recent log entries"""
        entries = list(self.buffer)[-count:]
        if level:
            entries = [e for e in entries if e.level == level]
        return entries

    def get_errors(self) -> List[LogEntry]:
        """Get all error entries from buffer"""
        return [e for e in self.buffer if e.level == "Error"]

    def get_warnings(self) -> List[LogEntry]:
        """Get all warning entries from buffer"""
        return [e for e in self.buffer if e.level == "Warning"]

    def save_filtered_log(self, path: Path):
        """Save filtered log entries to file"""
        with open(path, 'w') as f:
            for entry in self.buffer:
                if self.filter.matches(entry):
                    f.write(f"{entry.raw_line}\n")

    @property
    def stats(self) -> dict:
        """Get monitoring statistics"""
        return self._stats.copy()


class ConsoleLogPrinter:
    """Prints log entries to console (simple format for agent parsing)"""

    def __init__(self, show_timestamp: bool = False, show_category: bool = True):
        self.show_timestamp = show_timestamp
        self.show_category = show_category

    def __call__(self, entry: LogEntry):
        """Print a log entry in simple format"""
        parts = []
        if self.show_timestamp:
            parts.append(entry.timestamp.strftime('%H:%M:%S'))
        parts.append(entry.level)
        if self.show_category:
            parts.append(entry.category)
        parts.append(entry.message)

        print("|".join(parts))

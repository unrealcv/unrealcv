"""
UnrealCV Debug Harness - Configuration
Centralized configuration for build, test, and log monitoring.
"""
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Optional
import json
import os


@dataclass
class UEConfig:
    """Unreal Engine project configuration"""
    # UE Installation
    ue_path: Path = Path("H:/UE_5.6/Engine")

    # Project paths
    project_path: Path = Path("G:/HUAWEI_Project_UE56/HUAWEI_Project.uproject")
    plugin_root: Path = Path("G:/HUAWEI_Project_UE56/Plugins/unrealcv")

    # Network
    port: int = 9000
    host: str = "127.0.0.1"

    # Build settings
    build_timeout: int = 600  # seconds
    max_build_retries: int = 2

    # Launch settings
    launch_timeout: int = 180  # seconds
    server_ready_timeout: int = 60  # seconds
    post_launch_delay: float = 3.0  # seconds to wait after server ready before tests

    # Log settings
    log_filter_keywords: List[str] = field(default_factory=lambda: [
        "UnrealCV", "Error", "Warning", "Fatal", "Assert",
        "Blueprint", "Camera", "Sensor", "Recording"
    ])
    log_exclude_patterns: List[str] = field(default_factory=lambda: [
        "LogInit", "LogModuleManager", "LogStreamlineAPI"
    ])
    log_buffer_size: int = 1000  # lines to keep in memory
    log_output_dir: Path = field(default_factory=lambda: Path("./debug_logs"))

    # Test settings
    test_timeout: int = 300  # seconds
    test_patterns: List[str] = field(default_factory=lambda: [
        "test/server/*_test.py"
    ])

    @property
    def exe_path(self) -> Path:
        """Get game executable path"""
        project_name = self.project_path.stem
        return self.project_path.parent / "Binaries" / "Win64" / f"{project_name}.exe"

    @property
    def ue_log_path(self) -> Path:
        """Get UE log file path"""
        project_name = self.project_path.stem
        return self.project_path.parent / "Saved" / "Logs" / f"{project_name}.log"

    @property
    def build_tool_path(self) -> Optional[Path]:
        """Find available build tool"""
        patterns = [
            "Binaries/DotNET/UnrealBuildTool/UnrealBuildTool.exe",  # Prefer .exe on Windows
            "Build/BatchFiles/RunUAT.bat",
        ]
        for pattern in patterns:
            path = self.ue_path / pattern
            if path.exists():
                return path
        return None

    @classmethod
    def from_file(cls, path: Path) -> "UEConfig":
        """Load configuration from JSON file"""
        if path.exists():
            with open(path, 'r') as f:
                data = json.load(f)
                # Convert path strings to Path objects
                for key in ['ue_path', 'project_path', 'plugin_root', 'log_output_dir']:
                    if key in data:
                        data[key] = Path(data[key])
                return cls(**data)
        return cls()

    def save(self, path: Path):
        """Save configuration to JSON file"""
        data = {
            'ue_path': str(self.ue_path),
            'project_path': str(self.project_path),
            'plugin_root': str(self.plugin_root),
            'port': self.port,
            'host': self.host,
            'build_timeout': self.build_timeout,
            'max_build_retries': self.max_build_retries,
            'launch_timeout': self.launch_timeout,
            'server_ready_timeout': self.server_ready_timeout,
            'log_filter_keywords': self.log_filter_keywords,
            'log_exclude_patterns': self.log_exclude_patterns,
            'log_buffer_size': self.log_buffer_size,
            'log_output_dir': str(self.log_output_dir),
            'test_timeout': self.test_timeout,
            'test_patterns': self.test_patterns,
        }
        path.parent.mkdir(parents=True, exist_ok=True)
        with open(path, 'w') as f:
            json.dump(data, f, indent=2)


# Global config instance
_config: Optional[UEConfig] = None


def get_config() -> UEConfig:
    """Get global configuration instance"""
    global _config
    if _config is None:
        config_path = Path(__file__).parent / "config.json"
        _config = UEConfig.from_file(config_path)
    return _config


def set_config(config: UEConfig):
    """Set global configuration instance"""
    global _config
    _config = config

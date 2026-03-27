"""
UnrealCV Debug Harness - Configuration
Centralized configuration for build, test, and log monitoring.
"""
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Optional, Tuple
import json
import os


def _detect_paths_from_file() -> Tuple[Path, Path]:
    config_file = Path(__file__).resolve()

    # config.py -> workflow -> unrealcv (plugin_root)
    plugin_root = config_file.parent.parent
    assert plugin_root.name.startswith("unrealcv"), f"Expected 'unrealcv' folder, found '{plugin_root.name}' at {plugin_root}"

    # unrealcv -> Plugins -> ProjectRoot
    project_root = plugin_root.parent.parent

    # 查找 .uproject 文件
    uproject_files = list(project_root.glob("*.uproject"))
    assert uproject_files, f"No .uproject file found in {project_root}"

    project_path = uproject_files[0]

    return plugin_root, project_path


def _normalize_engine_path(path_value: Optional[str]) -> Optional[Path]:
    if not path_value:
        return None

    path = Path(path_value)
    if path.name.lower() == "engine":
        return path
    return path / "Engine"


def _read_engine_association(project_path: Path) -> Optional[str]:
    try:
        with open(project_path, "r", encoding="utf-8") as f:
            project_data = json.load(f)
    except (OSError, json.JSONDecodeError):
        return None

    association = project_data.get("EngineAssociation")
    if not association:
        return None
    return str(association)


def _candidate_engine_paths(engine_association: Optional[str]) -> List[Path]:
    candidates: List[Path] = []

    def add_candidate(path: Optional[Path]):
        if path and path not in candidates:
            candidates.append(path)

    add_candidate(_normalize_engine_path(os.getenv("UE_PATH")))

    if engine_association:
        version_suffixes = [engine_association]
        if engine_association.replace(".", "").isdigit():
            version_suffixes.append(engine_association.replace(".", "_"))

        for version in version_suffixes:
            for drive in ["M", "H", "D", "C"]:
                add_candidate(Path(f"{drive}:/UE_{version}/Engine"))
                add_candidate(Path(f"{drive}:/EpicGames/UE_{version}/Engine"))
                add_candidate(Path(f"{drive}:/Program Files/Epic Games/UE_{version}/Engine"))
            add_candidate(Path(f"C:/Program Files/Epic Games/UE_{version}/Engine"))

    return candidates


def _resolve_ue_path(project_path: Path, configured_path: Optional[Path]) -> Optional[Path]:
    if configured_path:
        normalized = _normalize_engine_path(str(configured_path))
        if normalized and normalized.exists():
            return normalized

    engine_association = _read_engine_association(project_path)
    for candidate in _candidate_engine_paths(engine_association):
        if candidate.exists():
            return candidate

    return configured_path


@dataclass
class UEConfig:
    """Unreal Engine project configuration"""
    # UE Installation (prefer UE_PATH/config.json, otherwise auto-detect from .uproject)
    ue_path: Optional[Path] = field(default_factory=lambda: _normalize_engine_path(os.getenv("UE_PATH")))

    # Project paths (auto-detected from __file__, fallback to env vars)
    project_path: Path = None
    plugin_root: Path = None

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
        "LogInit", "LogModuleManager", "LogStreamlineAPI", "Connection"
    ])
    log_include_levels: List[str] = field(default_factory=lambda: [
        "Error", "Fatal"
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

    if _config.project_path is None or _config.plugin_root is None:
        _config.plugin_root, _config.project_path = _detect_paths_from_file()
    _config.ue_path = _resolve_ue_path(_config.project_path, _config.ue_path)
    return _config


def set_config(config: UEConfig):
    """Set global configuration instance"""
    global _config
    _config = config

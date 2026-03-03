"""Pydantic v2 data models for the unrealcv Python client.

These models replace the ad-hoc ``list[float]`` / ``dict`` structures used
throughout the API with validated, documented types that integrate with
modern Python tooling (IDE autocomplete, JSON serialisation, FastAPI, etc.).

All models use ``model_config = ConfigDict(frozen=True)`` so instances are
hashable and safe to use as dict keys or in sets.

Example
-------
>>> from unrealcv.models import Pose, Color, Resolution
>>> pose = Pose(x=100, y=-200, z=300, roll=0, yaw=45, pitch=0)
>>> pose.location  # (100.0, -200.0, 300.0)
>>> color = Color(r=255, g=128, b=64)
>>> resolution = Resolution(width=1920, height=1080)
"""

from __future__ import annotations

from pydantic import BaseModel, ConfigDict, Field


class Color(BaseModel):
    """RGB colour with each channel in [0, 255]."""

    model_config = ConfigDict(frozen=True)

    r: int = Field(ge=0, le=255, description="Red channel")
    g: int = Field(ge=0, le=255, description="Green channel")
    b: int = Field(ge=0, le=255, description="Blue channel")

    def to_list(self) -> list[int]:
        """Return ``[r, g, b]`` for compatibility with legacy API."""
        return [self.r, self.g, self.b]

    @classmethod
    def from_list(cls, rgb: list[int]) -> Color:
        """Create from ``[r, g, b]`` list."""
        return cls(r=rgb[0], g=rgb[1], b=rgb[2])


class Vector3(BaseModel):
    """3-D vector (location or scale in Unreal world units)."""

    model_config = ConfigDict(frozen=True)

    x: float = 0.0
    y: float = 0.0
    z: float = 0.0

    def to_list(self) -> list[float]:
        return [self.x, self.y, self.z]

    @classmethod
    def from_list(cls, values: list[float]) -> Vector3:
        return cls(x=values[0], y=values[1], z=values[2])


class Rotator(BaseModel):
    """Euler rotation in degrees (Unreal convention: roll, yaw, pitch)."""

    model_config = ConfigDict(frozen=True)

    roll: float = 0.0
    yaw: float = 0.0
    pitch: float = 0.0

    def to_list(self) -> list[float]:
        return [self.roll, self.yaw, self.pitch]

    @classmethod
    def from_list(cls, values: list[float]) -> Rotator:
        return cls(roll=values[0], yaw=values[1], pitch=values[2])


class Pose(BaseModel):
    """Full 6-DoF pose = location + rotation."""

    model_config = ConfigDict(frozen=True)

    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    roll: float = 0.0
    yaw: float = 0.0
    pitch: float = 0.0

    @property
    def location(self) -> tuple[float, float, float]:
        return (self.x, self.y, self.z)

    @property
    def rotation(self) -> tuple[float, float, float]:
        return (self.roll, self.yaw, self.pitch)

    def to_list(self) -> list[float]:
        """Return ``[x, y, z, roll, yaw, pitch]`` for legacy API compat."""
        return [self.x, self.y, self.z, self.roll, self.yaw, self.pitch]

    @classmethod
    def from_list(cls, values: list[float]) -> Pose:
        return cls(
            x=values[0], y=values[1], z=values[2],
            roll=values[3], yaw=values[4], pitch=values[5],
        )


class Resolution(BaseModel):
    """Image resolution in pixels."""

    model_config = ConfigDict(frozen=True)

    width: int = Field(gt=0, description="Width in pixels")
    height: int = Field(gt=0, description="Height in pixels")

    def to_tuple(self) -> tuple[int, int]:
        return (self.width, self.height)

    @classmethod
    def from_tuple(cls, wh: tuple[int, int]) -> Resolution:
        return cls(width=wh[0], height=wh[1])


class BoundingBox(BaseModel):
    """Axis-aligned bounding box in pixel coordinates."""

    model_config = ConfigDict(frozen=True)

    x: int = Field(description="Top-left X")
    y: int = Field(description="Top-left Y")
    width: int = Field(ge=0, description="Box width")
    height: int = Field(ge=0, description="Box height")

    def to_list(self) -> list[int]:
        """Return ``[x, y, width, height]``."""
        return [self.x, self.y, self.width, self.height]

    @classmethod
    def from_list(cls, values: list[int]) -> BoundingBox:
        return cls(x=values[0], y=values[1], width=values[2], height=values[3])


class CameraConfig(BaseModel):
    """Per-camera configuration used by ``UnrealCv_API.cam`` dict."""

    model_config = ConfigDict(frozen=True)

    location: Vector3 = Field(default_factory=Vector3)
    rotation: Rotator = Field(default_factory=Rotator)

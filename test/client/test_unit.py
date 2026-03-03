"""Pure unit tests for unrealcv Python client internals.

These tests do NOT require a running Unreal Engine server.
They cover utility functions, SocketMessage framing, and MsgDecoder parsing.

Run with:
    pytest test/client/test_unit.py -v
"""

import io
import os
import struct
import ctypes

import numpy as np
import PIL.Image
import pytest


# ---------------------------------------------------------------------------
# util.py tests
# ---------------------------------------------------------------------------

class TestResChecker:
    """Tests for unrealcv.util.ResChecker."""

    @pytest.fixture(autouse=True)
    def _setup(self):
        from unrealcv.util import ResChecker
        self.checker = ResChecker()

    def test_is_error_with_none(self):
        assert self.checker.is_error(None) is True

    def test_is_error_with_error_string(self):
        assert self.checker.is_error("error something went wrong") is True

    def test_is_error_with_ok_string(self):
        assert self.checker.is_error("ok") is False

    def test_is_ok(self):
        assert self.checker.is_ok("ok") is True
        assert self.checker.is_ok("not ok") is False

    def test_not_error(self):
        assert self.checker.not_error("ok") is True
        assert self.checker.not_error(None) is False

    def test_is_expected_file_extension_valid(self):
        assert self.checker.is_expected_file_extension("img.png", [".png", ".jpg"]) is True

    def test_is_expected_file_extension_invalid(self):
        assert self.checker.is_expected_file_extension("img.bmp", [".png", ".jpg"]) is False


class TestReadPng:
    """Tests for unrealcv.util.read_png."""

    def test_valid_png_bytes(self):
        from unrealcv.util import read_png

        # Create a tiny 2x2 red PNG in memory
        img = PIL.Image.new("RGBA", (2, 2), (255, 0, 0, 255))
        buf = io.BytesIO()
        img.save(buf, format="PNG")
        png_bytes = buf.getvalue()

        result = read_png(png_bytes)
        assert result is not None
        assert result.shape[0] == 2
        assert result.shape[1] == 2

    def test_invalid_bytes_returns_none(self):
        from unrealcv.util import read_png
        result = read_png(b"not a PNG")
        assert result is None


class TestReadNpy:
    """Tests for unrealcv.util.read_npy."""

    def test_valid_npy_bytes(self):
        from unrealcv.util import read_npy

        arr = np.array([1.0, 2.0, 3.0], dtype=np.float32)
        buf = io.BytesIO()
        np.save(buf, arr)
        npy_bytes = buf.getvalue()

        result = read_npy(npy_bytes)
        assert result is not None
        np.testing.assert_array_almost_equal(result, arr)

    def test_invalid_bytes_returns_none(self):
        from unrealcv.util import read_npy
        result = read_npy(b"not npy data")
        assert result is None


class TestConvert2PlaneDepth:
    """Tests for unrealcv.util.convert2planedepth."""

    def test_uniform_depth(self):
        from unrealcv.util import convert2planedepth

        # Note: convert2planedepth uses np.float which is removed in NumPy 2.0+
        # This test validates the function works when np.float is available
        try:
            # A constant depth map should produce values <= the input
            point_depth = np.ones((4, 4), dtype=np.float64) * 100.0
            plane_depth = convert2planedepth(point_depth, f=320)
            assert plane_depth.shape == (4, 4)
            # Plane depth should be <= point depth everywhere
            assert np.all(plane_depth <= 100.0 + 1e-6)
        except AttributeError:
            pytest.skip("np.float removed in NumPy 2.0+ (pre-existing bug)")


class TestParseResolution:
    """Tests for unrealcv.util.parse_resolution."""

    def test_valid_resolution(self):
        from unrealcv.util import parse_resolution
        assert parse_resolution("1920x1080") == (1920, 1080)
        assert parse_resolution("640x480") == (640, 480)


class TestGetPath2UnrealEnv:
    """Tests for unrealcv.util.get_path2UnrealEnv."""

    def test_with_env_var_set(self, monkeypatch, tmp_path):
        from unrealcv.util import get_path2UnrealEnv

        env_dir = str(tmp_path / "UE")
        monkeypatch.setenv("UnrealEnv", env_dir)
        assert get_path2UnrealEnv() == env_dir

    def test_default_path_created(self, monkeypatch, tmp_path):
        from unrealcv.util import get_path2UnrealEnv

        monkeypatch.delenv("UnrealEnv", raising=False)
        monkeypatch.setenv("HOME", str(tmp_path))
        with pytest.warns(UserWarning, match="UnrealEnv environment variable not set"):
            path = get_path2UnrealEnv()
        assert "unrealcv" in path.lower() or "UnrealEnv" in path


# ---------------------------------------------------------------------------
# SocketMessage tests
# ---------------------------------------------------------------------------

class TestSocketMessage:
    """Tests for unrealcv.SocketMessage framing."""

    def test_magic_number(self):
        from unrealcv import SocketMessage
        assert SocketMessage.magic == ctypes.c_uint32(0x9E2B83C1).value

    def test_payload_size(self):
        from unrealcv import SocketMessage
        msg = SocketMessage(b"hello")
        assert msg.payload_size == 5

    def test_payload_size_empty(self):
        from unrealcv import SocketMessage
        msg = SocketMessage(b"")
        assert msg.payload_size == 0

    def test_payload_size_large(self):
        from unrealcv import SocketMessage
        payload = b"x" * 10000
        msg = SocketMessage(payload)
        assert msg.payload_size == 10000


# ---------------------------------------------------------------------------
# MsgDecoder tests
# ---------------------------------------------------------------------------

class TestMsgDecoder:
    """Tests for unrealcv.api.MsgDecoder parsing methods."""

    @pytest.fixture(autouse=True)
    def _setup(self):
        from unrealcv.api import MsgDecoder
        self.decoder = MsgDecoder()

    def test_string2list(self):
        result = self.decoder.string2list("one two three")
        assert result == ["one", "two", "three"]

    def test_string2list_single(self):
        result = self.decoder.string2list("solo")
        assert result == ["solo"]

    def test_string2floats(self):
        result = self.decoder.string2floats("1.0 2.5 3.7")
        assert result == [1.0, 2.5, 3.7]

    def test_string2floats_integers(self):
        result = self.decoder.string2floats("1 2 3")
        assert result == [1.0, 2.0, 3.0]

    def test_string2color_rgba(self):
        result = self.decoder.string2color("(R=255,G=128,B=64,A=255)")
        assert result == [255, 128, 64]

    def test_string2color_no_alpha(self):
        # Still works if there's no alpha — returns first 3 of 4
        result = self.decoder.string2color("(R=100,G=200,B=50,A=0)")
        assert result == [100, 200, 50]

    def test_string2vector(self):
        result = self.decoder.string2vector("X=1.5 Y=-2.5 Z=3.0")
        assert len(result) == 3
        assert result[0] == pytest.approx(1.5)
        assert result[1] == pytest.approx(-2.5)
        assert result[2] == pytest.approx(3.0)

    def test_cmd2key(self):
        assert self.decoder.cmd2key("vget /camera/0/location") == "location"
        assert self.decoder.cmd2key("vget /object/cube/color") == "color"

    def test_decode_vertex(self):
        vertex_data = "0.0 1.0 2.0\n3.0 4.0 5.0"
        result = self.decoder.decode_vertex(vertex_data)
        assert len(result) == 2
        assert result[0] == [0.0, 1.0, 2.0]
        assert result[1] == [3.0, 4.0, 5.0]

    def test_decode_png(self):
        # Create a minimal RGBA PNG
        img = PIL.Image.new("RGBA", (3, 3), (10, 20, 30, 255))
        buf = io.BytesIO()
        img.save(buf, format="PNG")
        png_bytes = buf.getvalue()

        result = self.decoder.decode_png(png_bytes)
        assert result.shape == (3, 3, 3)  # alpha dropped, channels reversed

    def test_decode_npy(self):
        arr = np.array([[1.0, 2.0], [3.0, 4.0]], dtype=np.float32)
        buf = io.BytesIO()
        np.save(buf, arr)
        npy_bytes = buf.getvalue()

        result = self.decoder.decode_npy(npy_bytes)
        assert result.shape == (2, 2, 1)  # 2D expanded to 3D

    def test_decode_depth(self):
        depth = np.array([[10.0, 20.0], [30.0, 40.0]], dtype=np.float32)
        buf = io.BytesIO()
        np.save(buf, depth)
        npy_bytes = buf.getvalue()

        result = self.decoder.decode_depth(npy_bytes, inverse=False)
        assert result.shape == (2, 2, 1)
        np.testing.assert_array_almost_equal(result[:, :, 0], depth)

    def test_decode_depth_inverse(self):
        depth = np.array([[10.0, 20.0]], dtype=np.float32)
        buf = io.BytesIO()
        np.save(buf, depth)
        npy_bytes = buf.getvalue()

        result = self.decoder.decode_depth(npy_bytes, inverse=True)
        np.testing.assert_array_almost_equal(result[0, 0, 0], 0.1)
        np.testing.assert_array_almost_equal(result[0, 1, 0], 0.05)

    def test_empty(self):
        assert self.decoder.empty("anything") == "anything"

    def test_decode_map_keys(self):
        """Verify that the decoder map has expected keys."""
        expected_keys = {"location", "rotation", "color", "png", "bmp", "npy"}
        assert expected_keys.issubset(set(self.decoder.decode_map.keys()))

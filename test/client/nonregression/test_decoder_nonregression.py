import io

import numpy as np
import pytest

from unrealcv.api import MsgDecoder


@pytest.fixture
def decoder():
    return MsgDecoder()


def _assert_single_pixel(image, expected):
    assert image.shape == (1, 1, 3)
    np.testing.assert_array_equal(image[0, 0], np.array(expected, dtype=np.uint8))


@pytest.mark.parametrize(
    "cmd,expected",
    [
        ("vget /object/cube/color", "color"),
        ("vget /camera/0/depth npy", "npy"),
        ("vget /camera/0/location", "location"),
    ],
    ids=["object-color", "camera-depth-mode", "camera-location"],
)
def test_msgdecoder_cmd2key_variants(decoder, cmd, expected):
    assert decoder.cmd2key(cmd) == expected


def test_msgdecoder_decode_color(decoder):
    color = decoder.decode("vget /object/cube/color", "(R=255,G=128,B=64,A=255)")
    assert color == [255, 128, 64]


def test_decode_bmp_raises_when_opencv_cannot_decode(monkeypatch, decoder):
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: None)
    with pytest.raises(ValueError, match="Failed to decode image data"):
        decoder.decode_bmp(b"invalid-image-bytes")


def test_decode_png_drops_alpha_and_swaps_to_bgr(make_png_rgba_bytes, decoder):
    payload = make_png_rgba_bytes((10, 20, 30, 255))

    image = decoder.decode_png(payload)

    _assert_single_pixel(image, [30, 20, 10])


@pytest.mark.parametrize(
    "array,expected_shape",
    [
        (np.array([[1.0, 2.0]], dtype=np.float32), (1, 2, 1)),
        (np.zeros((1, 2, 3), dtype=np.float32), (1, 2, 3)),
    ],
    ids=["2d-expands-channel", "3d-preserved"],
)
def test_decode_npy_shape_contract(decoder, make_npy_bytes, array, expected_shape):
    payload = make_npy_bytes(array)

    decoded = decoder.decode_npy(payload)

    assert decoded.shape == expected_shape


def test_decode_bmp_strips_alpha_channel(monkeypatch, decoder):
    bgra = np.array([[[1, 2, 3, 255]]], dtype=np.uint8)
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: bgra)

    decoded = decoder.decode_bmp(b"bmp-bytes")

    _assert_single_pixel(decoded, [1, 2, 3])


def test_decode_bmp_keeps_grayscale_shape(monkeypatch, decoder):
    gray = np.array([[7]], dtype=np.uint8)
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: gray)

    decoded = decoder.decode_bmp(b"bmp-bytes")

    assert decoded.shape == (1, 1)


def test_decode_img_unknown_mode_raises_value_error(decoder):
    with pytest.raises(ValueError, match="Unknown image mode"):
        decoder.decode_img(b"data", mode="jpg")


@pytest.mark.parametrize(
    "decoder_fn,payload",
    [
        ("decode_png", b"not-a-png"),
        ("decode_npy", b"not-a-npy"),
        ("decode_depth", b"not-a-depth"),
    ],
    ids=["png-invalid", "npy-invalid", "depth-invalid"],
)
def test_decoder_invalid_bytes_raise(decoder, decoder_fn, payload):
    with pytest.raises(Exception):
        getattr(decoder, decoder_fn)(payload)


def test_decode_depth_inverse_applies_and_expands_channel(decoder):
    arr = np.array([[2.0, 4.0]], dtype=np.float32)
    buf = io.BytesIO()
    np.save(buf, arr)

    depth = decoder.decode_depth(buf.getvalue(), inverse=True)

    assert depth.shape == (1, 2, 1)
    np.testing.assert_allclose(
        depth[:, :, 0], np.array([[0.5, 0.25]], dtype=np.float32)
    )

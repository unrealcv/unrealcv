import pytest

from unrealcv.api import MsgDecoder
from unrealcv import Client


def test_batch_cmd_decodes_each_response_with_kwargs(dummy_client_factory, api_factory):
    client = dummy_client_factory([["1 2 3", "4 5 6"]])
    api = api_factory(client)

    def decode_with_offset(res, offset):
        return [float(value) + offset for value in res.split()]

    result = api.batch_cmd(
        cmds=["vget /camera/0/location", "vget /camera/0/rotation"],
        decoders=[decode_with_offset, decode_with_offset],
        offset=0.5,
    )

    assert result == [[1.5, 2.5, 3.5], [4.5, 5.5, 6.5]]


def test_batch_cmd_returns_raw_when_decoder_list_is_none(
    dummy_client_factory, api_factory
):
    client = dummy_client_factory([["ok", "ok"]])
    api = api_factory(client)

    result = api.batch_cmd(cmds=["vset /foo", "vset /bar"], decoders=None)

    assert result == ["ok", "ok"]


def test_get_image_depth_return_cmd_uses_depth_endpoint(api_factory):
    api = api_factory()

    result = api.get_image(cam_id=3, viewmode="depth", return_cmd=True)

    assert result == "vget /camera/3/depth npy"


def test_get_image_non_depth_requests_and_decodes(
    monkeypatch, dummy_client_factory, api_factory
):
    client = dummy_client_factory([b"raw-image"])
    api = api_factory(client)
    captured = {}

    def fake_decode_img(res, mode, inverse):
        captured["res"] = res
        captured["mode"] = mode
        captured["inverse"] = inverse
        return "decoded-image"

    monkeypatch.setattr(api.decoder, "decode_img", fake_decode_img)

    result = api.get_image(cam_id=1, viewmode="lit", mode="png", inverse=True)

    assert result == "decoded-image"
    assert client.calls == [("vget /camera/1/lit png", ())]
    assert captured == {"res": b"raw-image", "mode": "png", "inverse": True}


def test_get_cam_location_retries_until_response_and_syncs_cache(
    dummy_client_factory, api_factory
):
    client = dummy_client_factory([None, "10 20 30"])
    api = api_factory(client)

    result = api.get_cam_location(0)

    assert result == [10.0, 20.0, 30.0]
    assert api.cam[0]["location"] == [10.0, 20.0, 30.0]
    assert [call[0] for call in client.calls] == [
        "vget /camera/0/location",
        "vget /camera/0/location",
    ]


def test_set_cam_fov_short_circuits_when_unchanged(dummy_client_factory, api_factory):
    client = dummy_client_factory([])
    api = api_factory(client)

    result = api.set_cam_fov(0, 90)

    assert result == 90
    assert client.calls == []


def test_decode_bmp_raises_when_opencv_cannot_decode(monkeypatch):
    decoder = MsgDecoder()

    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: None)

    with pytest.raises(ValueError, match="Failed to decode image data"):
        decoder.decode_bmp(b"invalid-image-bytes")


def test_client_connect_without_confirm_does_not_leave_connected_socket(
    monkeypatch, fake_socket_factory
):
    fake_socket = fake_socket_factory()

    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)
    monkeypatch.setattr("unrealcv.SocketMessage.ReceivePayload", lambda _sock: None)

    client = Client(("localhost", 1))
    connected = client.connect(timeout=0.1)

    assert connected is False
    assert client.isconnected() is False
    assert fake_socket.closed is True


def test_client_request_returns_none_when_send_fails(monkeypatch):
    client = Client(("localhost", 1))

    monkeypatch.setattr(client, "send", lambda _message: False)

    assert client.request("vget /camera/0/location") is None

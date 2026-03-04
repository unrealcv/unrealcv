import numpy as np
import pytest

from unrealcv import Client
from unrealcv.api import MsgDecoder


@pytest.mark.parametrize(
    "cmd,expected",
    [
        ("vget /object/cube/color", "color"),
        ("vget /camera/0/depth npy", "npy"),
        ("vget /camera/0/location", "location"),
    ],
)
def test_msgdecoder_cmd2key_variants(cmd, expected):
    decoder = MsgDecoder()
    assert decoder.cmd2key(cmd) == expected


def test_msgdecoder_decode_color():
    decoder = MsgDecoder()
    color = decoder.decode("vget /object/cube/color", "(R=255,G=128,B=64,A=255)")
    assert color == [255, 128, 64]


def test_decode_bmp_raises_when_opencv_cannot_decode(monkeypatch):
    decoder = MsgDecoder()
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: None)
    with pytest.raises(ValueError, match="Failed to decode image data"):
        decoder.decode_bmp(b"invalid-image-bytes")


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
    assert api.batch_cmd(cmds=["vset /foo", "vset /bar"], decoders=None) == ["ok", "ok"]


@pytest.mark.parametrize(
    "cam_id,viewmode,mode,expected",
    [
        (3, "depth", "bmp", "vget /camera/3/depth npy"),
        (1, "lit", "png", "vget /camera/1/lit png"),
        (2, "normal", "bmp", "vget /camera/2/normal bmp"),
    ],
)
def test_get_image_return_cmd_routing(api_factory, cam_id, viewmode, mode, expected):
    api = api_factory()
    assert (
        api.get_image(cam_id=cam_id, viewmode=viewmode, mode=mode, return_cmd=True)
        == expected
    )


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


def test_get_image_multicam_uses_batch_with_expected_commands_and_kwargs(
    monkeypatch, api_factory
):
    api = api_factory()
    captured = {}

    def fake_batch(cmds, decoders, **kwargs):
        captured["cmds"] = cmds
        captured["decoders"] = decoders
        captured["kwargs"] = kwargs
        return ["img0", "img1"]

    monkeypatch.setattr(api, "batch_cmd", fake_batch)

    result = api.get_image_multicam([0, 2], viewmode="lit", mode="png", inverse=False)

    assert result == ["img0", "img1"]
    assert captured["cmds"] == ["vget /camera/0/lit png", "vget /camera/2/lit png"]
    assert len(captured["decoders"]) == 2
    assert captured["kwargs"] == {"mode": "png", "inverse": False}


def test_get_image_multimodal_concatenates_modalities_in_order(
    monkeypatch, api_factory
):
    api = api_factory()
    rgb = np.zeros((2, 2, 3), dtype=np.uint8)
    depth = np.ones((2, 2, 1), dtype=np.float32)

    monkeypatch.setattr(api, "batch_cmd", lambda *_args, **_kwargs: [rgb, depth])

    result = api.get_image_multimodal(
        0, viewmodes=["lit", "depth"], modes=["bmp", "npy"]
    )

    assert result.shape == (2, 2, 4)
    np.testing.assert_array_equal(result[:, :, :3], rgb)
    np.testing.assert_array_equal(result[:, :, 3:], depth)


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


@pytest.mark.parametrize(
    "initial_fov,target_fov,expected_calls",
    [
        (90, 90, []),
        (90, 120, [("vset /camera/0/fov 120", (-1,))]),
    ],
)
def test_set_cam_fov_behavior(
    dummy_client_factory, api_factory, initial_fov, target_fov, expected_calls
):
    client = dummy_client_factory([])
    api = api_factory(client)
    api.cam[0]["fov"] = initial_fov

    result = api.set_cam_fov(0, target_fov)

    assert result == target_fov
    assert client.calls == expected_calls


def test_save_image_issues_same_command_twice_and_returns_second_response(
    dummy_client_factory, api_factory
):
    client = dummy_client_factory(["ignored", "/tmp/out.png"])
    api = api_factory(client)

    result = api.save_image(0, "lit", "frame.png")

    assert result == "/tmp/out.png"
    assert client.calls == [
        ("vget /camera/0/lit frame.png", ()),
        ("vget /camera/0/lit frame.png", ()),
    ]


def test_get_cam_pose_soft_mode_mutates_cached_location_list(api_factory):
    api = api_factory()
    api.cam[0]["location"] = [1.0, 2.0, 3.0]
    api.cam[0]["rotation"] = [4.0, 5.0, 6.0]

    pose = api.get_cam_pose(0, mode="soft")

    assert pose == [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
    assert api.cam[0]["location"] == [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]


def test_get_cam_pose_hard_mode_updates_cache_from_batch(monkeypatch, api_factory):
    api = api_factory()
    monkeypatch.setattr(
        api,
        "batch_cmd",
        lambda *_args, **_kwargs: [[10.0, 20.0, 30.0], [1.0, 2.0, 3.0]],
    )

    pose = api.get_cam_pose(0, mode="hard")

    assert pose == [10.0, 20.0, 30.0, 1.0, 2.0, 3.0]
    assert api.cam[0]["location"] == [10.0, 20.0, 30.0]
    assert api.cam[0]["rotation"] == [1.0, 2.0, 3.0]


@pytest.mark.parametrize("handshake_payload", [None, b"rejected"])
def test_client_connect_without_valid_confirm_does_not_leave_connected_socket(
    monkeypatch, fake_socket_factory, handshake_payload
):
    fake_socket = fake_socket_factory()

    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)
    monkeypatch.setattr(
        "unrealcv.SocketMessage.ReceivePayload", lambda _sock: handshake_payload
    )

    client = Client(("localhost", 1))
    connected = client.connect(timeout=0.1)

    assert connected is False
    assert client.isconnected() is False
    assert fake_socket.closed is True


def test_client_request_returns_none_when_send_fails(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: False)
    assert client.request("vget /camera/0/location") is None


def test_client_request_list_delegates_to_request_batch(monkeypatch):
    client = Client(("localhost", 1))
    calls = {}

    def fake_request_batch(batch):
        calls["batch"] = batch
        return ["ok1", "ok2"]

    monkeypatch.setattr(client, "request_batch", fake_request_batch)

    assert client.request(["cmd1", "cmd2"]) == ["ok1", "ok2"]
    assert calls["batch"] == ["cmd1", "cmd2"]


@pytest.mark.parametrize(
    "message,expected_attr", [("cmd", "single"), (["cmd1", "cmd2"], "batch")]
)
def test_client_request_async_routes_to_expected_path(
    monkeypatch, message, expected_attr
):
    client = Client(("localhost", 1))
    calls = {"single": 0, "batch": 0}

    monkeypatch.setattr(
        client,
        "request_async",
        lambda _msg: calls.__setitem__("single", calls["single"] + 1),
    )
    monkeypatch.setattr(
        client,
        "request_batch_async",
        lambda _msgs: calls.__setitem__("batch", calls["batch"] + 1),
    )

    result = client.request(message, timeout=-1)

    assert result is True
    assert calls[expected_attr] == 1

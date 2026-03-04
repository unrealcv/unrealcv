import numpy as np
import pytest


def _assert_plain_request_calls(client, commands):
    assert client.calls == [(command, ()) for command in commands]


def _assert_cam_pose_cache(api, location, rotation):
    assert api.cam[0]["location"] == location
    assert api.cam[0]["rotation"] == rotation


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
    assert api.batch_cmd(cmds=["vset /foo", "vset /bar"], decoders=None) == [
        "ok",
        "ok",
    ]


@pytest.mark.parametrize(
    "cam_id,viewmode,mode,expected",
    [
        (3, "depth", "bmp", "vget /camera/3/depth npy"),
        (1, "lit", "png", "vget /camera/1/lit png"),
        (2, "normal", "bmp", "vget /camera/2/normal bmp"),
    ],
    ids=["depth-always-npy", "lit-png", "normal-bmp"],
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
    _assert_plain_request_calls(client, ["vget /camera/1/lit png"])
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
    assert [call[0] for call in client.calls] == ["vget /camera/0/location"] * 2


@pytest.mark.parametrize(
    "initial_fov,target_fov,expected_calls",
    [
        (90, 90, []),
        (90, 120, [("vset /camera/0/fov 120", (-1,))]),
    ],
    ids=["unchanged-fov", "changed-fov"],
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
    _assert_plain_request_calls(
        client,
        ["vget /camera/0/lit frame.png", "vget /camera/0/lit frame.png"],
    )


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
    _assert_cam_pose_cache(api, [10.0, 20.0, 30.0], [1.0, 2.0, 3.0])

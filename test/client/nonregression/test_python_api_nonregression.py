import io
import queue
import struct

import numpy as np
import pytest

from unrealcv import Client, SocketMessage
from unrealcv.api import MsgDecoder


# MsgDecoder behavior locks
@pytest.mark.parametrize(
    "cmd,expected",
    [
        ("vget /object/cube/color", "color"),
        ("vget /camera/0/depth npy", "npy"),
        ("vget /camera/0/location", "location"),
    ],
    ids=["object-color", "camera-depth-mode", "camera-location"],
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


def test_decode_png_drops_alpha_and_swaps_to_bgr(make_png_rgba_bytes):
    decoder = MsgDecoder()
    payload = make_png_rgba_bytes((10, 20, 30, 255))

    image = decoder.decode_png(payload)

    assert image.shape == (1, 1, 3)
    np.testing.assert_array_equal(image[0, 0], np.array([30, 20, 10], dtype=np.uint8))


@pytest.mark.parametrize(
    "array,expected_shape",
    [
        (np.array([[1.0, 2.0]], dtype=np.float32), (1, 2, 1)),
        (np.zeros((1, 2, 3), dtype=np.float32), (1, 2, 3)),
    ],
    ids=["2d-expands-channel", "3d-preserved"],
)
def test_decode_npy_shape_contract(make_npy_bytes, array, expected_shape):
    decoder = MsgDecoder()
    payload = make_npy_bytes(array)

    decoded = decoder.decode_npy(payload)

    assert decoded.shape == expected_shape


def test_decode_bmp_strips_alpha_channel(monkeypatch):
    decoder = MsgDecoder()
    bgra = np.array([[[1, 2, 3, 255]]], dtype=np.uint8)
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: bgra)

    decoded = decoder.decode_bmp(b"bmp-bytes")

    assert decoded.shape == (1, 1, 3)
    np.testing.assert_array_equal(decoded[0, 0], np.array([1, 2, 3], dtype=np.uint8))


def test_decode_bmp_keeps_grayscale_shape(monkeypatch):
    decoder = MsgDecoder()
    gray = np.array([[7]], dtype=np.uint8)
    monkeypatch.setattr("unrealcv.api.cv2.imdecode", lambda *_args, **_kwargs: gray)

    decoded = decoder.decode_bmp(b"bmp-bytes")

    assert decoded.shape == (1, 1)


def test_decode_img_unknown_mode_raises_value_error():
    decoder = MsgDecoder()
    with pytest.raises(ValueError, match="Unknown image mode"):
        decoder.decode_img(b"data", mode="jpg")


# UnrealCv_API behavior locks


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


# Client transport behavior locks
@pytest.mark.parametrize(
    "handshake_payload",
    [None, b"rejected"],
    ids=["no-handshake", "non-connected-handshake"],
)
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
    with pytest.raises(ConnectionError, match="socket is closed"):
        client.request("vget /camera/0/location")


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
    "message,expected_attr",
    [("cmd", "single"), (["cmd1", "cmd2"], "batch")],
    ids=["single-async", "batch-async"],
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


@pytest.mark.parametrize("method_name", ["request_async", "request_batch_async"])
def test_client_async_methods_raise_when_send_fails(monkeypatch, method_name):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: False)

    with pytest.raises(ConnectionError, match="socket is closed"):
        if method_name == "request_async":
            client.request_async("vget /camera/0/location")
        else:
            client.request_batch_async(["cmd1", "cmd2"])


def test_client_request_batch_raises_when_send_fails(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: False)

    with pytest.raises(ConnectionError, match="socket is closed"):
        client.request_batch(["cmd1"])


def test_client_request_sync_queues_single_result_request(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("ok")

    result = client.request("vget /camera/0/location")

    assert result == "ok"
    assert client.recv_num_q.get() == -1


def test_client_request_sync_encodes_text_with_message_id(monkeypatch):
    client = Client(("localhost", 1))
    sent_messages = []

    def fake_send(message):
        sent_messages.append(message)
        return True

    monkeypatch.setattr(client, "send", fake_send)
    client.recv_data_q.put("ok")

    result = client.request("héllo")

    assert result == "ok"
    assert sent_messages[0].startswith(b"0:")
    assert sent_messages[0].endswith("héllo".encode("utf-8"))


def test_client_request_sync_increments_send_message_id(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("ok")

    _ = client.request("cmd")

    assert client.send_message_id == 1


def test_client_request_raises_timeout_error_when_response_queue_times_out(monkeypatch):
    class _TimeoutQueue:
        def get(self, timeout=None):
            raise queue.Empty

    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q = _TimeoutQueue()

    with pytest.raises(TimeoutError, match="Request timed out"):
        client.request("vget /camera/0/location", timeout=0.01)


def test_client_request_batch_queues_negative_batch_size(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("r1")
    client.recv_data_q.put("r2")

    result = client.request_batch(["cmd1", "cmd2"])

    assert result == ["r1", "r2"]
    assert client.recv_num_q.get() == -2


def test_client_request_batch_raises_when_response_queue_contains_exception(monkeypatch):
    client = Client(("localhost", 1))
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put(ConnectionError("server error"))

    with pytest.raises(ConnectionError, match="server error"):
        client.request_batch(["cmd1"])


def test_receive_loop_queue_sync_path_decodes_and_enqueues(monkeypatch):
    client = Client(("localhost", 1))
    raw_messages = [b"0:ok"]
    monkeypatch.setattr(client, "receive", lambda: raw_messages.pop(0))

    client.recv_num_q.put(-1)
    client.recv_num_q.put(None)
    client.receive_loop_queue()

    assert client.recv_message_id == 1
    assert client.recv_data_q.get() == "ok"


def test_receive_loop_queue_async_path_does_not_enqueue_data(monkeypatch):
    client = Client(("localhost", 1))
    raw_messages = [b"0:ok"]
    monkeypatch.setattr(client, "receive", lambda: raw_messages.pop(0))

    client.recv_num_q.put(1)
    client.recv_num_q.put(None)
    client.receive_loop_queue()

    assert client.recv_message_id == 1
    assert client.recv_data_q.empty() is True


def test_receive_abnormal_disconnect_retries_then_asserts(monkeypatch):
    client = Client(("localhost", 1))
    client.sock = object()
    reconnect_attempts = {"count": 0}
    disconnect_calls = {"count": 0}

    monkeypatch.setattr("unrealcv.SocketMessage.ReceivePayload", lambda _sock: None)
    monkeypatch.setattr(
        client,
        "disconnect",
        lambda: disconnect_calls.__setitem__("count", disconnect_calls["count"] + 1),
    )
    monkeypatch.setattr(
        client,
        "connect",
        lambda *args, **kwargs: (
            reconnect_attempts.__setitem__("count", reconnect_attempts["count"] + 1)
            or False
        ),
    )
    monkeypatch.setattr("unrealcv.time.sleep", lambda _seconds: None)

    with pytest.raises(ConnectionError, match="Failed to reconnect"):
        client.receive()

    assert reconnect_attempts["count"] == client.RECONNECT_ATTEMPTS
    assert disconnect_calls["count"] == 1


def test_raw_message_handler_malformed_payload_returns_none():
    client = Client(("localhost", 1))
    assert client.raw_message_handler(b"malformed") is None


def test_raw_message_handler_message_id_mismatch_asserts():
    client = Client(("localhost", 1))
    client.recv_message_id = 1
    with pytest.raises(AssertionError):
        client.raw_message_handler(b"0:ok")


def test_raw_message_handler_decodes_utf8_payload():
    client = Client(("localhost", 1))
    client.recv_message_id = 0
    assert client.raw_message_handler("0:héllo".encode("utf-8")) == "héllo"


def test_raw_message_handler_keeps_binary_payload_when_not_utf8():
    client = Client(("localhost", 1))
    client.recv_message_id = 0
    payload = b"\x89PNG\x00"
    assert client.raw_message_handler(b"0:" + payload) == payload


def test_send_connected_wraps_payload(monkeypatch):
    client = Client(("localhost", 1))
    client.sock = object()
    sent = {}

    def fake_wrap(sock, message):
        sent["sock"] = sock
        sent["message"] = message
        return True

    monkeypatch.setattr("unrealcv.SocketMessage.WrapAndSendPayload", fake_wrap)

    assert client.send(b"payload") is True
    assert sent == {"sock": client.sock, "message": b"payload"}


def test_send_disconnected_returns_false():
    client = Client(("localhost", 1))
    assert client.send(b"payload") is False


def test_connect_success_sets_socket_and_starts_receive_thread(
    monkeypatch, fake_socket_factory
):
    class _FakeThread:
        def __init__(self, target, daemon=False):
            self.target = target
            self.daemon = daemon
            self.started = False

        def start(self):
            self.started = True

        def is_alive(self):
            return self.started

        def join(self):
            self.started = False

    fake_socket = fake_socket_factory()
    created_threads = []

    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)
    monkeypatch.setattr(
        "unrealcv.SocketMessage.ReceivePayload", lambda _sock: b"connected to test"
    )

    def fake_thread_factory(*args, **kwargs):
        thread = _FakeThread(*args, **kwargs)
        created_threads.append(thread)
        return thread

    monkeypatch.setattr("unrealcv.threading.Thread", fake_thread_factory)

    client = Client(("localhost", 1))
    connected = client.connect()

    assert connected is True
    assert client.sock is fake_socket
    assert len(created_threads) == 1
    assert created_threads[0].target == client.receive_loop_queue
    assert created_threads[0].started is True


def test_connect_with_alive_receive_thread_does_not_spawn_another(
    monkeypatch, fake_socket_factory
):
    class _AliveThread:
        def is_alive(self):
            return True

    fake_socket = fake_socket_factory()
    created_threads = []

    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)
    monkeypatch.setattr(
        "unrealcv.SocketMessage.ReceivePayload", lambda _sock: b"connected to test"
    )
    monkeypatch.setattr(
        "unrealcv.threading.Thread",
        lambda *args, **kwargs: created_threads.append((args, kwargs)),
    )

    client = Client(("localhost", 1))
    client.t = _AliveThread()

    connected = client.connect()

    assert connected is True
    assert created_threads == []


def test_connect_with_unsupported_socket_type_returns_false():
    client = Client(("localhost", 1), type="invalid")
    assert client.connect() is False


def test_connect_closes_socket_when_connect_raises(monkeypatch, fake_socket_factory):
    fake_socket = fake_socket_factory()

    def fail_connect(_endpoint):
        raise OSError("connect failed")

    fake_socket.connect = fail_connect
    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)

    client = Client(("localhost", 1))
    assert client.connect() is False
    assert fake_socket.closed is True


def test_disconnect_joins_alive_receive_thread(monkeypatch):
    class _AliveThread:
        def __init__(self):
            self.joined = False

        def is_alive(self):
            return True

        def join(self, timeout=None):
            self.joined = True

    client = Client(("localhost", 1))
    thread = _AliveThread()
    client.t = thread

    client.disconnect()

    assert client.recv_num_q.get() is None
    assert thread.joined is True


def test_disconnect_closes_socket_even_when_shutdown_raises(monkeypatch):
    class _SocketWithShutdownError:
        def __init__(self):
            self.close_called = False

        def shutdown(self, _mode):
            raise OSError("shutdown failed")

        def close(self):
            self.close_called = True

    client = Client(("localhost", 1))
    sock = _SocketWithShutdownError()
    client.sock = sock
    monkeypatch.setattr("unrealcv.time.sleep", lambda _seconds: None)

    client.disconnect()

    assert sock.close_called is True
    assert client.sock is None


@pytest.mark.parametrize(
    "decoder_fn,payload",
    [
        ("decode_png", b"not-a-png"),
        ("decode_npy", b"not-a-npy"),
        ("decode_depth", b"not-a-depth"),
    ],
    ids=["png-invalid", "npy-invalid", "depth-invalid"],
)
def test_decoder_invalid_bytes_raise(decoder_fn, payload):
    decoder = MsgDecoder()
    with pytest.raises(Exception):
        getattr(decoder, decoder_fn)(payload)


def test_decode_depth_inverse_applies_and_expands_channel():
    decoder = MsgDecoder()
    arr = np.array([[2.0, 4.0]], dtype=np.float32)
    buf = io.BytesIO()
    np.save(buf, arr)

    depth = decoder.decode_depth(buf.getvalue(), inverse=True)

    assert depth.shape == (1, 2, 1)
    np.testing.assert_allclose(
        depth[:, :, 0], np.array([[0.5, 0.25]], dtype=np.float32)
    )


class _ScriptedRecvSocket:
    def __init__(self, chunks):
        self._chunks = list(chunks)

    def recv(self, _size):
        if self._chunks:
            return self._chunks.pop(0)
        return b""

    def recv_into(self, buffer, nbytes=0):
        size = nbytes or len(buffer)
        chunk = self.recv(size)
        read_size = len(chunk)
        buffer[:read_size] = chunk
        return read_size


def test_socketmessage_receivepayload_handles_partial_reads():
    payload = b"abc"
    magic = struct.pack("I", SocketMessage.magic)
    size = struct.pack("I", len(payload))
    sock = _ScriptedRecvSocket([magic[:2], magic[2:], size, payload[:1], payload[1:]])

    result = SocketMessage.ReceivePayload(sock)

    assert result == payload


def test_socketmessage_receivepayload_rejects_invalid_magic():
    bad_magic = struct.pack("I", 0)
    size = struct.pack("I", 3)
    sock = _ScriptedRecvSocket([bad_magic, size, b"abc"])

    result = SocketMessage.ReceivePayload(sock)

    assert result is None


def test_socketmessage_receivepayload_zero_length_payload_returns_empty_bytes():
    magic = struct.pack("I", SocketMessage.magic)
    size = struct.pack("I", 0)
    sock = _ScriptedRecvSocket([magic, size])

    result = SocketMessage.ReceivePayload(sock)

    assert result == b""


def test_socketmessage_receivepayload_short_size_header_raises_struct_error():
    magic = struct.pack("I", SocketMessage.magic)
    sock = _ScriptedRecvSocket([magic, b"\x01\x00"])

    assert SocketMessage.ReceivePayload(sock) is None


def test_socketmessage_receivepayload_truncated_payload_returns_none():
    payload = b"abc"
    magic = struct.pack("I", SocketMessage.magic)
    size = struct.pack("I", len(payload))
    sock = _ScriptedRecvSocket([magic, size, b"a", b""])

    result = SocketMessage.ReceivePayload(sock)

    assert result is None


class _FailingWriteSocket:
    def sendall(self, _data):
        raise RuntimeError("boom")


def test_socketmessage_wrap_send_payload_returns_false_on_write_error():
    result = SocketMessage.WrapAndSendPayload(_FailingWriteSocket(), b"payload")
    assert result is False


class _RecordingFile:
    def __init__(self):
        self.buffer = b""

    def write(self, data):
        self.buffer += data


class _RecordingWriteSocket:
    def __init__(self):
        self.file = _RecordingFile()

    def sendall(self, data):
        self.file.write(data)


def test_socketmessage_wrap_send_payload_writes_magic_size_and_payload():
    sock = _RecordingWriteSocket()
    payload = b"xyz"

    ok = SocketMessage.WrapAndSendPayload(sock, payload)

    assert ok is True
    assert sock.file.buffer[:4] == struct.pack("I", SocketMessage.magic)
    assert sock.file.buffer[4:8] == struct.pack("I", len(payload))
    assert sock.file.buffer[8:] == payload


def test_socketmessage_receivepayload_returns_none_on_recv_exception():
    class _ExceptionSocket:
        def recv_into(self, _buffer, _nbytes=0):
            raise RuntimeError("boom")

    assert SocketMessage.ReceivePayload(_ExceptionSocket()) is None

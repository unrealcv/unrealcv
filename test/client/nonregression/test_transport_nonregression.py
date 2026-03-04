import queue
import struct

import pytest

from unrealcv import Client, SocketMessage


LOCAL_ENDPOINT = ("localhost", 1)


def _new_client():
    return Client(LOCAL_ENDPOINT)


def _patch_socket_and_handshake(monkeypatch, fake_socket, handshake_payload):
    monkeypatch.setattr("unrealcv.socket.socket", lambda *_args, **_kwargs: fake_socket)
    monkeypatch.setattr(
        "unrealcv.SocketMessage.ReceivePayload", lambda _sock: handshake_payload
    )


@pytest.mark.parametrize(
    "handshake_payload",
    [None, b"rejected"],
    ids=["no-handshake", "non-connected-handshake"],
)
def test_client_connect_without_valid_confirm_does_not_leave_connected_socket(
    monkeypatch, fake_socket_factory, handshake_payload
):
    fake_socket = fake_socket_factory()
    _patch_socket_and_handshake(monkeypatch, fake_socket, handshake_payload)

    client = _new_client()
    connected = client.connect(timeout=0.1)

    assert connected is False
    assert client.isconnected() is False
    assert fake_socket.closed is True


def test_client_request_raises_connection_error_when_send_fails(monkeypatch):
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: False)
    with pytest.raises(ConnectionError, match="socket is closed"):
        client.request("vget /camera/0/location")


def test_client_request_list_delegates_to_request_batch(monkeypatch):
    client = _new_client()
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
    client = _new_client()
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
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: False)

    with pytest.raises(ConnectionError, match="socket is closed"):
        if method_name == "request_async":
            client.request_async("vget /camera/0/location")
        else:
            client.request_batch_async(["cmd1", "cmd2"])


def test_client_request_batch_raises_when_send_fails(monkeypatch):
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: False)

    with pytest.raises(ConnectionError, match="socket is closed"):
        client.request_batch(["cmd1"])


def test_client_request_sync_queues_single_result_request(monkeypatch):
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("ok")

    result = client.request("vget /camera/0/location")

    assert result == "ok"
    assert client.recv_num_q.get() == -1


def test_client_request_sync_encodes_text_with_message_id(monkeypatch):
    client = _new_client()
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
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("ok")

    _ = client.request("cmd")

    assert client.send_message_id == 1


def test_client_request_raises_timeout_error_when_response_queue_times_out(monkeypatch):
    class _TimeoutQueue:
        def get(self, timeout=None):
            raise queue.Empty

    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q = _TimeoutQueue()

    with pytest.raises(TimeoutError, match="Request timed out"):
        client.request("vget /camera/0/location", timeout=0.01)


def test_client_request_batch_queues_negative_batch_size(monkeypatch):
    client = _new_client()
    monkeypatch.setattr(client, "send", lambda _message: True)
    client.recv_data_q.put("r1")
    client.recv_data_q.put("r2")

    result = client.request_batch(["cmd1", "cmd2"])

    assert result == ["r1", "r2"]
    assert client.recv_num_q.get() == -2


def test_client_request_batch_raises_when_response_queue_contains_exception(
    monkeypatch,
):
    client = _new_client()
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
    _patch_socket_and_handshake(monkeypatch, fake_socket, b"connected to test")

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
    _patch_socket_and_handshake(monkeypatch, fake_socket, b"connected to test")
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


def _framed_payload_chunks(payload):
    magic = struct.pack("I", SocketMessage.magic)
    size = struct.pack("I", len(payload))
    return magic, size, payload


def test_socketmessage_receivepayload_handles_partial_reads():
    payload = b"abc"
    magic, size, payload = _framed_payload_chunks(payload)
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
    magic, size, _ = _framed_payload_chunks(b"")
    sock = _ScriptedRecvSocket([magic, size])

    result = SocketMessage.ReceivePayload(sock)

    assert result == b""


def test_socketmessage_receivepayload_short_size_header_raises_struct_error():
    magic, _, _ = _framed_payload_chunks(b"")
    sock = _ScriptedRecvSocket([magic, b"\x01\x00"])

    assert SocketMessage.ReceivePayload(sock) is None


def test_socketmessage_receivepayload_truncated_payload_returns_none():
    magic, size, _ = _framed_payload_chunks(b"abc")
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

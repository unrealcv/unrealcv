"""
Use python dummy server to test the robustness of unrealcv.Client, Pass the test of DevServer first
"""

import unittest, threading, random, logging, time, sys, socket
import unrealcv
from dev_server import EchoServer, MessageServer, NullServer
import pytest

# Configure the logging level of this test script
logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
# Use NullHandler to avoid logging issues when pytest closes stdout during teardown
logger.addHandler(logging.NullHandler())

# Configure unrealcv logging verbose level
logging.getLogger("unrealcv").setLevel(logging.CRITICAL)
logging.getLogger("dev_server").setLevel(logging.DEBUG)


def random_payload():
    len = random.randrange(1024)
    random_str = "".join([chr(random.randrange(100)) for v in range(len)])
    return random_str


localhost = "localhost"


def _find_free_port(host=localhost):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.bind((host, 0))
        return sock.getsockname()[1]


@pytest.fixture(scope="function")
def echo_port():
    return _find_free_port()


@pytest.fixture(scope="function")
def server(echo_port):
    server = MessageServer((localhost, echo_port))
    server.start()  # Wait until the server is started
    yield server
    server.shutdown()


def test_request(server, echo_port):
    """Simple test for basic functions"""
    client = unrealcv.Client((localhost, echo_port))
    cmds = ["hi", "hello", "asdf" * 70]
    client.connect()
    assert client.isconnected() == True
    for cmd in cmds:
        res = client.request(cmd)
        assert res == cmd
    client.disconnect()  # TODO: What if forgot to disconnect
    # server.shutdown()


def test_multi_connection(server, echo_port):
    """
    Only one client is allowed for the server
    Make a second connection to the server, when one connection exists
    """
    # server = MessageServer((localhost, echo_port))
    # server.start()
    client = unrealcv.Client((localhost, echo_port))
    client.connect(timeout=0.1)
    assert client.isconnected() == True
    response = client.request("hi")
    assert response == "hi"
    for i in range(10):
        _client = unrealcv.Client((localhost, echo_port))
        _client.connect(0.1)
        # print client.connect()
        assert _client.isconnected() == False
    client.disconnect()
    # server.shutdown()


# @pytest.mark.skip(reason = 'This test will fail in windows for an unknown reason.')
def test_client_release(server, echo_port):
    """
    If the previous client release the connection, further connection should be accepted. This will also test the server code
    """
    # server = MessageServer((localhost, echo_port))
    # server.start()
    client = unrealcv.Client((localhost, echo_port))

    num_release_trial = 3
    logger.info("Try to release client %d times" % num_release_trial)
    for i in range(num_release_trial):
        msg = "Running trial %d" % i
        client.connect()
        assert client.isconnected() == True, (
            "Client is not successfully connected in trial %d" % i
        )

        # Do something
        req = "ok"
        res = client.request(req)
        assert req == res, msg

        client.disconnect()
        # Make sure the server can correctly handle the disconnection signal
        assert client.isconnected() == False, (
            "Client is not successfully disconnected in trial %d" % i
        )
        # Make sure the server can detect client connection also
        time.sleep(0.5)  # Wait for the server to detect the disconnection
        assert server.get_client_socket() == None, (
            "Server can not detect client disconnect event."
        )
        logger.info("Trial %d is finished." % i)
    # server.shutdown()


def test_random_operation(server, echo_port):
    """Randomly connect and disconnect the client, this is very likely to fail"""
    # server = MessageServer((localhost, echo_port))
    # server.start()
    client = unrealcv.Client((localhost, echo_port))

    num_random_trial = 3
    print("Try random operation %d times" % num_random_trial)
    for i in range(num_random_trial):
        msg = "Trial %d" % i
        choice = random.randrange(2)
        if choice == 1:
            client.connect()
            assert client.isconnected() == True, msg
        elif choice == 0:
            client.disconnect()
            assert client.isconnected() == False, msg

    for i in range(3):
        client.connect()
        assert client.isconnected() == True
    for i in range(3):
        client.disconnect()
        assert client.isconnected() == False
    # server.shutdown()


def test_request_timeout():
    """What if the server did not respond with a correct reply."""

    null_port = _find_free_port()
    null_server = NullServer((localhost, null_port))
    null_server.start()

    client = unrealcv.Client((localhost, null_port))
    client.connect()
    assert client.isconnected() == True
    with pytest.raises(TimeoutError):
        client.request("hi", timeout=1)
    null_server.shutdown()


def test_no_server():
    """What if server is not started yet?"""

    no_port = _find_free_port()
    client = unrealcv.Client((localhost, no_port), None)
    client.connect()
    assert client.isconnected() == False
    cmds = ["hi", "hello"]
    for cmd in cmds:
        with pytest.raises(ConnectionError):
            client.request(cmd)


def test_disconnect_from_receive_thread_no_self_join(echo_port):
    """disconnect() should not try joining the current receive thread."""
    client = unrealcv.Client((localhost, echo_port))
    client.t = threading.current_thread()
    client.disconnect()


def test_receive_reconnect_does_not_spawn_extra_thread(monkeypatch, echo_port):
    """receive() reconnect path should call connect() without spawning another receive thread."""
    client = unrealcv.Client((localhost, echo_port))
    client.sock = object()

    sentinel_thread = object()
    client.t = sentinel_thread
    connect_calls = []
    payloads = [None, b"0:ok"]

    def fake_receive_payload(_sock):
        return payloads.pop(0)

    def fake_disconnect():
        client.sock = None

    def fake_connect(timeout=1, start_receive_thread=True):
        connect_calls.append((timeout, start_receive_thread))
        client.sock = object()
        return True

    monkeypatch.setattr(
        unrealcv.SocketMessage, "ReceivePayload", staticmethod(fake_receive_payload)
    )
    monkeypatch.setattr(client, "disconnect", fake_disconnect)
    monkeypatch.setattr(client, "connect", fake_connect)

    raw = client.receive()

    assert raw == b"0:ok"
    assert connect_calls
    assert connect_calls[0][1] is False
    assert client.t is sentinel_thread


def test_receive_reconnect_failure_propagates_error(monkeypatch, echo_port):
    """On reconnect exhaustion, receive() should raise and also publish an error for waiters."""
    client = unrealcv.Client((localhost, echo_port))
    client.sock = object()
    client.RECONNECT_ATTEMPTS = 2
    client.RECONNECT_BASE_DELAY = 0
    client.RECONNECT_MAX_DELAY = 0

    monkeypatch.setattr(
        unrealcv.SocketMessage,
        "ReceivePayload",
        staticmethod(lambda _sock: None),
    )
    monkeypatch.setattr(client, "disconnect", lambda: setattr(client, "sock", None))
    monkeypatch.setattr(
        client, "connect", lambda timeout=1, start_receive_thread=True: False
    )

    with pytest.raises(ConnectionError):
        client.receive()

    queued_error = client.recv_data_q.get(timeout=0.2)
    assert isinstance(queued_error, ConnectionError)


def test_server_shutdown():
    """Close on the server side and check whether client can detect it"""
    pass


def test_stress():
    """Create very large payload to see whether the connection is stable"""
    pass


def test_message_handler(server, echo_port):
    """Check message handler can correctly handle events from the server.
    And the thread is correctly handled that we can do something in the message_handler"""
    # echo_server = MessageServer((localhost, echo_port))
    # echo_server.start()
    client = unrealcv.Client((localhost, echo_port))

    def handle_message(msg):
        print("Got server message %s" % repr(msg))
        res = unrealcv.client.request("ok", 1)
        assert res == "ok"
        print("Server response %s" % res)

    client.connect()
    assert client.isconnected() == True
    client.message_handler = handle_message

    res = client.request("ok")
    assert res == "ok"

    server.send("Hello from server")
    # echo_server.shutdown()


def test_change_port():
    pass

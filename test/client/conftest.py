import pytest
import numpy as np
from io import BytesIO
import PIL.Image

from dev_server import MessageServer, NullServer
from unrealcv.api import MsgDecoder, UnrealCv_API
from unrealcv.util import ResChecker


# API test doubles
class DummyClient:
    def __init__(self, responses=None):
        self._responses = list(responses or [])
        self.calls = []

    def request(self, cmd, *args):
        self.calls.append((cmd, args))
        if self._responses:
            return self._responses.pop(0)
        return None


class FakeSocket:
    def __init__(self):
        self.connected = False
        self.closed = False
        self.connected_endpoint = None
        self.timeout = None
        self.shutdown_called = False

    def settimeout(self, timeout):
        self.timeout = timeout

    def connect(self, endpoint):
        self.connected = True
        self.connected_endpoint = endpoint

    def shutdown(self, _how):
        self.shutdown_called = True

    def close(self):
        self.closed = True


# API fixtures
@pytest.fixture
def dummy_client_factory():
    """Factory for scripted client responses used by API unit tests."""

    def _factory(responses=None):
        return DummyClient(responses)

    return _factory


@pytest.fixture
def fake_socket_factory():
    """Factory for fake socket objects used in handshake failure tests."""

    def _factory():
        return FakeSocket()

    return _factory


@pytest.fixture
def api_factory(dummy_client_factory):
    """Build UnrealCv_API instances without running network-heavy __init__."""

    def _factory(client=None):
        api = UnrealCv_API.__new__(UnrealCv_API)
        api.decoder = MsgDecoder()
        api.checker = ResChecker()
        api.obj_dict = {}
        api.cam = {
            0: {"location": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "fov": 90}
        }
        api.client = client or dummy_client_factory()
        return api

    return _factory


# Network fixtures
@pytest.fixture(scope="session")
def localhost():
    return "localhost"


@pytest.fixture(scope="session")
def free_port_factory(localhost):
    """Allocate an ephemeral localhost TCP port for tests."""
    import socket

    def _factory():
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.bind((localhost, 0))
            return sock.getsockname()[1]

    return _factory


@pytest.fixture(scope="module")
def echo_port(free_port_factory):
    return free_port_factory()


@pytest.fixture(scope="module")
def server(localhost, echo_port):
    """Start and tear down a shared MessageServer for module tests."""
    message_server = MessageServer((localhost, echo_port))
    message_server.start()
    yield message_server
    message_server.shutdown()


@pytest.fixture
def null_server_factory(localhost):
    """Create NullServer instances and guarantee cleanup after each test."""
    servers_to_cleanup = []

    def _factory(port):
        null_server = NullServer((localhost, port))
        null_server.start()
        servers_to_cleanup.append(null_server)
        return null_server

    yield _factory

    for null_server in servers_to_cleanup:
        null_server.shutdown()


# Binary payload fixtures
@pytest.fixture
def make_png_rgba_bytes():
    """Create in-memory RGBA PNG payloads for decoder tests."""

    def _factory(rgba_pixel=(10, 20, 30, 255)):
        image = PIL.Image.new("RGBA", (1, 1), rgba_pixel)
        buffer = BytesIO()
        image.save(buffer, format="PNG")
        return buffer.getvalue()

    return _factory


@pytest.fixture
def make_npy_bytes():
    """Serialize numpy arrays to in-memory NPY payload bytes."""

    def _factory(array):
        buffer = BytesIO()
        np.save(buffer, array)
        return buffer.getvalue()

    return _factory

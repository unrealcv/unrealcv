import unrealcv
from unrealcv import client

# from docker_util import docker_runner
import pytest
from conftest import checker


def test_connection():
    client.connect(timeout=2)
    res = client.request("vget /unrealcv/status", timeout=5)
    assert checker.not_error(res)
    # assert res == 'ok'


def test_echo():
    client.connect(timeout=2)
    res = client.request("vget /unrealcv/echo test", timeout=5)
    assert res == "test"


def test_viewmode():
    viewmodes = ["lit", "depth", "object_mask", "normal"]
    for viewmode in viewmodes:
        cmd = "vset /viewmode {viewmode}".format(viewmode=viewmode)
        print(cmd)
        res = client.request(cmd, timeout=5)
        assert checker.is_ok(res)
    client.disconnect()

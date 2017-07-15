from unrealcv import client
# from docker_util import docker_runner
import pytest
from conftest import checker

def test_connection():
    client.connect()
    res = client.request('vget /unrealcv/status')
    assert checker.not_error(res)
    # assert res == 'ok'

def test_echo():
    client.connect()
    res = client.request('vget /unrealcv/echo test')
    assert res == 'test'

def test_viewmode():
    viewmodes = ['lit', 'depth', 'object_mask', 'normal']
    for viewmode in viewmodes:
        cmd = 'vset /viewmode {viewmode}'.format(viewmode = viewmode)
        print(cmd)
        res = client.request(cmd)
        assert checker.is_ok(res)

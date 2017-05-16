from unrealcv import client
# from docker_util import docker_runner
import pytest
from conftest import env

def test_connection(env):
    client.connect()
    res = client.request('vget /unrealcv/status')
    # assert res == 'ok'

def test_viewmode(env):
    viewmodes = ['lit', 'depth', 'object_mask', 'normal']
    for viewmode in viewmodes:
        cmd = 'vset /viewmode {viewmode}'.format(viewmode = viewmode)
        res = client.request(cmd)
        assert res == 'ok'

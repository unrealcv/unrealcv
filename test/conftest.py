'''
Configuration file for pytest
'''
import pytest
import os, time
from docker_util import DockerRunner
from unrealcv import client

def pytest_addoption(parser):
    parser.addoption('--docker', action='store_true', help='Run docker fixture')

@pytest.fixture(scope='module')
def env(request):
    '''
    Return docker instance if use docker
    Return an empty environment if --no-docker is set
    '''
    if request.config.getoption('docker'):
        docker_cmd = '/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering'
        volumes = [(os.path.abspath('output'), '/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output')]
        # The path needs to be absolute
        runner = DockerRunner('qiuwch/rr', volumes)
        runner.start(docker_cmd)
        time.sleep(2)
        yield runner
        runner.stop()
    else:
        yield None


class ResChecker:
    # Define some utility functions to check whether the response is as expected
    def is_error(self, res):
        return (res is None) or res.startswith('error')

    def is_ok(self, res):
        return res == 'ok'

    def not_error(self, res):
        return not self.is_error(res)

checker = ResChecker()

def ver():
    client.connect()
    res = client.request('vget /unrealcv/version')
    client.connect()

    if res and res.startswith('error Can not find a handler'):
        return (0, 3, 0) # or earlier
    elif checker.is_error(res):
        print('Fail to connect to the game, make sure the game is running.')
        exit(-1)
    else:
        version = [int(v) for v in res.lstrip('v').split('.')]
        return tuple(version)

# ver = get_version()
# Version is represent as a python tuple (a, b, c)
# Comparing two tuples are done by comparing a, b, c in order

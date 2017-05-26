'''
Configuration file for pytest
'''
import pytest
import os, time
from docker_util import DockerRunner

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


# =============================
# Define some utility functions
def iserror(res):
    return (res is None) or res.startswith('error')

def isok(res):
    return res == 'ok'

class Version:
    '''
    Represent the version information, can be used for comparison
    '''
    def __init__(self, data):
        if isinstance(data, str):
            # parse vx.y.z
            [self.x, self.y, self.z] = [int(v) for v in data.lstrip('v').split('.')]

    def __cmp__(self, v):
        if self.x != v.x:
            return cmp(self.x, v.x)
        if self.y != v.y:
            return cmp(self.y, v.y)
        return cmp(self.z, v.z)

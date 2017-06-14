'''
Make a report showing the speed of some critical commands
Check whether it is fast enough for your application
'''
from __future__ import print_function
from unrealcv import client
import docker_util
import time
import pytest

def run_command(cmd, num):
    for _ in range(num):
        client.request(cmd)

if __name__ == '__main__':
    print('Start docker')
    # Initialize the environment
    docker_util.runner.start()
    client.connect()

    commands = [
        ('vget /unrealcv/status', 1000),
        ('vset /unrealcv/sync test message', 1000),
        ('vget /camera/0/lit', 100),
    ]

    for (cmd, num) in commands:
        tic = time.time()
        run_command(cmd, num)
        toc = time.time()
        elapse = toc - tic
        print('Run %s for %d, time = %.2f, %d FPS' % (cmd, num, elapse, float(num) / elapse))

    docker_util.runner.stop()

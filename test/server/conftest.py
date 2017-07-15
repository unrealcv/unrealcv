'''
Configuration file for pytest
'''
import pytest
import os, time
from unrealcv import client

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

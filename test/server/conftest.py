'''
Configuration file for pytest
'''
import socket

import pytest
from unrealcv import client


class ResChecker:
    # Define some utility functions to check whether the response is as expected
    def is_error(self, res):
        if res is None:
            return True
        elif type(res) == str:
            return res.startswith('error')
        else:  # res is npy array or Bytes
            return False

    def is_ok(self, res):
        return res == 'ok'

    def not_error(self, res):
        return not self.is_error(res)


checker = ResChecker()

_SERVER_AVAILABLE = True
_SERVER_UNAVAILABLE_REASON = ""


def _probe_server(timeout=2):
    try:
        with socket.create_connection(("localhost", 9000), timeout=timeout):
            return True, ""
    except OSError as e:
        return False, str(e)


def pytest_configure(config):
    global _SERVER_AVAILABLE, _SERVER_UNAVAILABLE_REASON
    _SERVER_AVAILABLE, _SERVER_UNAVAILABLE_REASON = _probe_server()


def pytest_collection_modifyitems(config, items):
    if _SERVER_AVAILABLE:
        return

    reason = (
        "UnrealCV server unavailable on localhost:9000"
        f" ({_SERVER_UNAVAILABLE_REASON})"
    )
    marker = pytest.mark.skip(reason=reason)
    for item in items:
        item.add_marker(marker)


def ver():
    if not _SERVER_AVAILABLE:
        return (0, 0, 0)

    try:
        if not client.connect(timeout=2):
            return (0, 0, 0)
        res = client.request('vget /unrealcv/version', timeout=2)
        if not res or checker.is_error(res):
            return (0, 0, 0)
        if res.startswith('error Can not find a handler'):
            return (0, 3, 0) # or earlier

        version = [int(v) for v in res.lstrip('v').split('.')]
        return tuple(version)
    except Exception:
        return (0, 0, 0)
    finally:
        try:
            client.disconnect()
        except Exception:
            pass


# ver = get_version()
# Version is represent as a python tuple (a, b, c)
# Comparing two tuples are done by comparing a, b, c in order

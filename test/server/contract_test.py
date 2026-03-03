"""Command contract tests for UnrealCV server.

These tests validate stable protocol/command behavior that client libraries depend on.
They intentionally focus on low-cost commands (`status`, `help`, `version`, `echo`)
so they can run quickly on any scene.
"""

from unrealcv import client
from conftest import checker


def test_status_contract():
    client.connect()
    res = client.request('vget /unrealcv/status')
    assert checker.not_error(res)
    assert 'Port:' in res
    assert 'Width:' in res
    assert 'Height:' in res


def test_help_contract():
    client.connect()
    res = client.request('vget /unrealcv/help')
    assert checker.not_error(res)
    assert 'vget /unrealcv/status' in res
    assert 'vget /unrealcv/version' in res


def test_version_contract():
    client.connect()
    res = client.request('vget /unrealcv/version')
    assert checker.not_error(res)
    assert res.startswith('v')


def test_echo_contract():
    client.connect()
    payload = 'contract_check'
    res = client.request(f'vget /unrealcv/echo {payload}')
    assert res == payload


def test_unknown_command_contract():
    client.connect()
    res = client.request('vget /unrealcv/this_command_does_not_exist')
    assert checker.is_error(res)
    assert 'Can not find a handler' in res

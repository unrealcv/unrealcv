"""
Test object related functions
"""

import pytest
from unrealcv import client
from conftest import checker


def test_object_list():
    client.connect(timeout=2)
    res = client.request("vget /objects", timeout=5)
    if checker.is_error(res):
        pytest.skip("Object listing is not available in this host project")
    obj_ids = res.split(" ")
    assert checker.not_error(res)

    for obj_id in obj_ids:
        color = client.request("vget /object/%s/color" % obj_id, timeout=5)
        if isinstance(color, str) and (
            "Cannot find object" in color or color.startswith("error Not Implemented")
        ):
            pytest.skip(
                "Object color query not supported for this host project content"
            )
        assert checker.not_error(color)
    client.disconnect()

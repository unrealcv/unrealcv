'''
Test object related functions
'''
from unrealcv import client
import cv2
from conftest import env, checker

def test_object_list(env):
    client.connect()
    res = client.request('vget /objects')
    obj_ids = res.split(' ')
    assert checker.not_error(res)

    for obj_id in obj_ids:
        color = client.request('vget /object/%s/color' % obj_id)
        assert checker.not_error(color)

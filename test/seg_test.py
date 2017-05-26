# pytest seg.py -k [name]
from unrealcv import client
import cv2
from common import isok, iserror


def test_seg_mask():
    client.connect()

    client.request('vrun setres 640x480')
    f1 = client.request('vget /camera/0/lit')
    f2 = client.request('vget /camera/0/object_mask')
    assert (not iserror(f1)) and (not iserror(f2))
    im = cv2.imread(f1)
    seg = cv2.imread(f2)
    assert(im.shape[:2] == seg.shape[:2]) # Make sure the width and height the same.

def test_object_list():
    client.connect()
    obj_ids = client.request('vget /objects').split(' ')

    for obj_id in obj_ids:
        color = client.request('vget /object/%s/color' % obj_id)
        print color

from unrealcv import client
from conftest import env
import cv2

def test_gt(env):
    client.connect()
    gt_types = ['lit', 'depth', 'normal', 'object_mask']
    for v in gt_types:
        res = client.request('vget /camera/0/{type} output/{type}.png'.format(type=v))
        print(res)
        im = cv2.imread(res)
        print(im.shape)

    # Make sure the dimension is right

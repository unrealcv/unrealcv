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

from PIL import Image
import io
def test_video_stream(env):
    client.connect()
    res = client.request('vget /camera/0/lit_raw')
    image = Image.open(io.BytesIO(res))
    # print image
    # print len(res)

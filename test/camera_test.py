from unrealcv import client
from conftest import env
import cv2
from PIL import Image

def test_gt(env):
    client.connect()
    gt_types = ['lit', 'depth', 'normal', 'object_mask']
    for v in gt_types:
        res = client.request('vget /camera/0/{type} output/{type}.png'.format(type=v))
        print(res)
        im = cv2.imread(res)
        print(im.shape)

    # Make sure the dimension is right

def imread_binary(res):
    import StringIO
    import numpy as np
    PILimg = Image.open(StringIO.StringIO(res))
    img = np.array(PILimg)
    return img

def imread(res):
    return cv2.imread(res)

def test_binary_mode(env):
    client.connect()
    cmds = [
        'vget /camera/0/lit png',
        'vget /camera/0/object_mask png',
        'vget /camera/0/normal png',
    ]
    for cmd in cmds:
        print('Test %s' % cmd)
        res = client.request(cmd)
        print(len(res))
        im = imread_binary(res)
        print im.shape

    # TODO: add assert

def test_file_mode(env):
    client.connect()
    print client.request('vget /unrealcv/status')
    res = client.request('vget /camera/0/lit test.png')
    im = imread(res)
    print im.shape

    res = client.request('vget /camera/0/object_mask test.png')
    im = imread(res)
    print im.shape

if __name__ == '__main__':
    test_binary_mode(None)
    # test_file_mode(None)

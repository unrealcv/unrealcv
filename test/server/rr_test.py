'''
Test to verify the RealisticRendering environment.
Because we already know a lot about the RealisticRendering environment, we assume some of its properties, such as object location, material color, etc. And use these as ground truth to verify the plugin.
'''
import pytest

@pytest.mark.skip(reason="Not implemented")
def test_seg_mask():
    client.connect()

    client.request('vrun setres 640x480')
    f1 = client.request('vget /camera/0/lit')
    f2 = client.request('vget /camera/0/object_mask')
    assert (not iserror(f1)) and (not iserror(f2))
    im = cv2.imread(f1)
    seg = cv2.imread(f2)
    assert(im.shape[:2] == seg.shape[:2]) # Make sure the width and height the same.

@pytest.mark.skip(reason="Not implemented")
def test_object_list():
    client.connect()
    obj_ids = client.request('vget /objects').split(' ')

    for obj_id in obj_ids:
        color = client.request('vget /object/%s/color' % obj_id)
        print(color)

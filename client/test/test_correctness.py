# Check whether the rendering result is as expected.
from common_conf import *
import time, os, ipdb
import ue4cv
import matplotlib.pyplot as plt

datafolder = 'correctness_test'

def render_frame(client, pos):
    loc = pos[0] # location
    rot = pos[1] # rotation
    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok', response
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok', response
    files = []
    # time.sleep(5)
    time.sleep(5)
    modes = ['lit', 'unlit', 'depth', 'object_mask']
    f = dict()
    for mode in modes:
        cmd = 'vget /camera/0/%s' % mode
        f[mode] = client.request(cmd)
    return f

def load_gt_files():
    '''
    Load a list of correctly rendered images and compare with images just rendered
    '''
    filename = os.path.join(datafolder, 'camera_info_basename.txt')
    with open(filename, 'r') as f:
        lines = f.readlines()
    # Parse camera location and rotation from file
    camera_pos = []
    for line_id in range(len(lines)):
        line = lines[line_id].strip() # Remove \n at the end
        if line_id % 3 == 0: # filename
            filename = os.path.join('correctness_test', line)
        elif line_id % 3 == 1: # location
            location = [float(v) for v in line.split(' ')]
        elif line_id % 3 == 2: # Rotation
            rotation = [float(v) for v in line.split(' ')]
            camera_pos.append((filename, location, rotation))
    return camera_pos

def subplot_image(index, im):
    import matplotlib.pyplot as plt
    if isinstance(im, str):
        im = plt.imread(im)
    plt.subplot(index)
    plt.imshow(im)
    plt.axis('off')

def plot_sidebyside(im1, im2):
    import matplotlib.pyplot as plt
    subplot_image(121, im1)
    subplot_image(122, im2)
    plt.show()

def test_correctness(client):
    # Load a list of camera location
    gt_files = load_gt_files()
    # gt_files = [gt_files[0]]

    for (filename, location, rotation) in gt_files:
        f = render_frame(client, [location, rotation])
        rendered_filename = f['lit']

        # plot_sidebyside(filename, rendered_filename)
        diff = diff_with_refimage(filename, rendered_filename)
        print diff.sum()
        # Set an emprical threshold to check the rendering correctness

def diff_with_refimage(im1, im2):
    if isinstance(im1, str):
        im1 = plt.imread(im1)
    if isinstance(im2, str):
        im2 = plt.imread(im2)

    # diff = abs(im1 - im2) # Only need to care the absolute value
    imdiff = abs(im1[:,:,0:3] - im2[:,:,0:3]) # Do not substract alpha
    return imdiff

if __name__ == '__main__':
    client = ue4cv.client
    client.connect()
    test_correctness(client)

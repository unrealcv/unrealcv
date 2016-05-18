# Check whether the rendering result is as expected.
from test_common import *
import time, os, ipdb
import matplotlib.pyplot as plt

def message_handler(message):
    # Ignore message from server
    return

def get_filelist(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    # Parse camera location and rotation from file
    camera_pos = []
    for line_id in range(len(lines)):
        line = lines[line_id].strip() # Remove \n at the end
        if line_id % 3 == 0: # filename
            filename = line
        elif line_id % 3 == 1: # location
            location = [float(v) for v in line.split(' ')]
        elif line_id % 3 == 2: # Rotation
            rotation = [float(v) for v in line.split(' ')]
            camera_pos.append((filename, location, rotation))
    return camera_pos

def render_frame(client, pos):
    loc = pos[0] # location
    rot = pos[1] # rotation
    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok'
    files = []
    cmd = 'vget /camera/0/image'
    return client.request(cmd)

def check_image_difference(reference_filename, rendered_filename):
    print reference_filename
    print rendered_filename
    im1 = plt.imread(reference_filename)
    im2 = plt.imread(rendered_filename)
    imdiff = abs(im1[:,:,0:3] - im2[:,:,0:3]) # Do not substract alpha
    assert((im1[:,:,3] == im2[:,:,3]).all())
    # imdiff
    print im1.shape
    print im2.shape
    print imdiff.shape

    fig1 = plt.figure(0)
    plt.imshow(im1)
    fig2 = plt.figure(1)
    plt.imshow(im2)
    fig3 = plt.figure(2)
    plt.imshow(imdiff)
    plt.set_cmap('hot')
    plt.tight_layout()
    [fig.canvas.draw() for fig in [fig1, fig2, fig3]]
    # [ax.axis('off') for ax in [ax1, ax2, ax3]]

image_id = 0
filelist = None
datafolder = 'correctness_test'

def press(event):
    global image_id
    if event.key == 'j':
        image_id -= 1
    elif event.key == 'k':
        image_id += 1
    else:
        return

    image_id = image_id % len(filelist)
    check_id(image_id)

def check_id(image_id):
    fileinfo = filelist[image_id]
    reference_filename = os.path.join(datafolder, fileinfo[0])
    camera_pos = fileinfo[1:]

    rendered_filename = render_frame(client, camera_pos)
    time.sleep(1) # TODO: Remove this sleep time

    check_image_difference(reference_filename, rendered_filename)

def test_correctness(client):
    # Load a list of camera location
    global filelist
    filelist = get_filelist(os.path.join(datafolder, 'camera_info_basename.txt'))

    # Check the difference in an interactive mode
    figs = [plt.figure(fig_id) for fig_id in range(3)]
    [fig.canvas.mpl_connect('key_press_event', press) for fig in figs]
    check_id(0)
    plt.show()

if __name__ == '__main__':
    client = Client((HOST, PORT), message_handler)
    test_correctness(client)

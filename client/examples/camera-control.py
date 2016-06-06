import sys
sys.path.append('..')
import ue4cv
import matplotlib.pyplot as plt
import numpy as np

# import ue4cv
plt.rcParams['keymap.save'] = ''
def onpress(event):
    print event.key
    cmd = ''
    if event.key == 'd':
        rot[1] += 1
        cmd = 'vset /camera/0/rotation %s' % ' '.join([str(v) for v in rot])

    if event.key == 'a':
        rot[1] -= 1
        cmd = 'vset /camera/0/rotation %s' % ' '.join([str(v) for v in rot])

    if event.key == 'q':
        loc[2] += 1
        cmd = 'vset /camera/0/location %s' % ' '.join([str(v) for v in loc])
    # cmd = 'vset /camera/0/location %s' % ' '.join([str(v) for v in pos])
    if cmd:
        print cmd
        ue4cv.client.request(cmd)

if __name__ == '__main__':
    ue4cv.client.connect()
    init_loc = [float(v) for v in ue4cv.client.request('vget /camera/0/location').split(' ')]
    init_rot = [float(v) for v in ue4cv.client.request('vget /camera/0/rotation').split(' ')]
    loc = init_loc; rot = init_rot
    image = np.zeros((300, 300))

    fig, ax = plt.subplots()
    fig.canvas.mpl_connect('key_press_event', onpress)
    ax.imshow(image)
    plt.axis('off')
    plt.show() # Add event handler

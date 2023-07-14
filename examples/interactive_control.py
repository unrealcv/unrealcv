# A toy example to use python to control the game.
from unrealcv import client
from unrealcv.util import read_npy, read_png
import matplotlib.pyplot as plt
import numpy as np


help_message = '''
A demo showing how to control a game using python
a, d: rotate camera to left and right.
q, e: move camera up and down.
left, right, up, down: move around
'''
plt.rcParams['keymap.save'] = ''

def main():
    loc = None
    rot = None

    fig, ax = plt.subplots()
    img = np.zeros((480, 640, 4))
    ax.imshow(img)

    def onpress(event):
        rot_offset = 10 # Rotate 5 degree for each key press
        loc_offset = 10 # Move 5.0 when press a key

        if event.key == 'a': rot[1] -= rot_offset
        if event.key == 'd': rot[1] += rot_offset
        if event.key == 'q': loc[2] += loc_offset # Move up
        if event.key == 'e': loc[2] -= loc_offset # Move down

        if event.key == 'w': loc[1] -= loc_offset
        if event.key == 's': loc[1] += loc_offset
        if event.key == 'up': loc[1] -= loc_offset
        if event.key == 'down': loc[1] += loc_offset
        if event.key == 'left': loc[0] -= loc_offset
        if event.key == 'right': loc[0] += loc_offset

        cmd = 'vset /camera/0/rotation %s' % ' '.join([str(v) for v in rot])
        client.request(cmd)
        cmd = 'vset /camera/0/location %s' % ' '.join([str(v) for v in loc])
        client.request(cmd)

        res = client.request('vget /camera/0/lit png')
        img = read_png(res)

        # print(event.key)
        # print('Requested image %s' % str(img.shape))

        ax.imshow(img)
        fig.canvas.draw()

    client.connect()
    if not client.isconnected():
        print ('UnrealCV server is not running. Run the game from http://unrealcv.github.io first.')
        return
    else:
        print (help_message)

    init_loc = [float(v) for v in client.request('vget /camera/0/location').split(' ')]
    init_rot = [float(v) for v in client.request('vget /camera/0/rotation').split(' ')]

    loc = init_loc; rot = init_rot

    fig.canvas.mpl_connect('key_press_event', onpress)
    plt.title('Keep this window in focus, it will be used to receive key press event')
    plt.axis('off')
    plt.show() # Add event handler

if __name__ == '__main__':
    main()

# A toy example to use python to control the game.
import sys
sys.path.append('..')
from unrealcv import client
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

    def onpress(event):
        print event.key
        if event.key == 'a': rot[1] += 1
        if event.key == 'd': rot[1] -= 1
        if event.key == 'q': loc[2] += 1 # Move up
        if event.key == 'e': loc[2] -= 1 # Move down

        if event.key == 'up': loc[1] -= 2
        if event.key == 'down': loc[1] += 2
        if event.key == 'left': loc[0] -= 2
        if event.key == 'right': loc[0] += 2

        cmd = 'vset /camera/0/rotation %s' % ' '.join([str(v) for v in rot])
        client.request(cmd)
        cmd = 'vset /camera/0/location %s' % ' '.join([str(v) for v in loc])
        client.request(cmd)


    client.connect()
    if not client.isconnected():
        print 'UnrealCV server is not running. Run the game from http://unrealcv.github.io first.'
        return
    else:
        print help_message

    init_loc = [float(v) for v in client.request('vget /camera/0/location').split(' ')]
    init_rot = [float(v) for v in client.request('vget /camera/0/rotation').split(' ')]

    loc = init_loc; rot = init_rot
    image = np.zeros((300, 300))

    fig, ax = plt.subplots()
    fig.canvas.mpl_connect('key_press_event', onpress)
    ax.imshow(image)
    plt.title('Keep this window in focus, used to receive key press event')
    plt.axis('off')
    plt.show() # Add event handler


if __name__ == '__main__':
    main()

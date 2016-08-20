import sys, os
sys.path.append('../..')
import ue4cv

# Try to connect our python client to the game
ue4cv.client.connect()

output_folder = 'unrealcv-files'

def f(filename):
    return os.path.join(output_folder, filename)

# Check if the connection is successfully established
if not ue4cv.client.isconnected():
    print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
else:
    filename = ue4cv.client.request('vget /camera/0/lit ' + f('lit.png'))
    print 'Image is saved to %s' % filename

    filename = ue4cv.client.request('vget /camera/0/depth ' + f('depth.png'))
    print 'Depth is saved to %s' % filename

    filename = ue4cv.client.request('vget /camera/0/depth ' + f('depth.exr'))
    print 'Depth is saved to %s' % filename

    filename = ue4cv.client.request('vget /camera/0/object_mask ' + f('object_mask.png'))
    print 'Object instance mask is saved to %s' % filename

    filename = ue4cv.client.request('vget /camera/0/debug ' + f('debug.exr'))
    print 'Object instance mask is saved to %s' % filename

    # filename = ue4cv.client.request('vget /camera/0/normal unrealcv/normal.png')
    # print 'Surface normal is saved to %s' % filename

    # Switch the camera mode back to normal
    res = ue4cv.client.request('vset /mode lit')
    assert(res == 'ok') # Make sure operation is successful

    # Disconnect python client from the game
    ue4cv.client.disconnect()

import sys, os
from test_cfg import client

# Try to connect our python client to the game
client.connect()
output_folder = 'unrealcv-files'

def f(mode, ext):
    return os.path.join(output_folder, '%s.%s' % (mode, ext))

# Check if the connection is successfully established
if not client.isconnected():
    print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
else:
    mode_list = [
        ('lit', 'png'),
        ('depth', 'png'),
        ('depth', 'exr'),
        ('object_mask', 'png'),
        ('debug', 'exr'),
    ]
    for (mode, ext) in mode_list:
        filename = client.request('vget /camera/0/%s %s' % (mode, f(mode, ext)))
        print '%s is saved to %s' % (mode, filename)

    # Switch the camera mode back to normal
    res = client.request('vset /viewmode lit')
    assert res == 'ok', res # Make sure operation is successful

    # Disconnect python client from the game
    client.disconnect()

# This is a 10 lines python example to show how to generate an image dataset with image, depth and object_mask.
# Read https://unrealcv.github.io/tutorial/getting_started.html before trying this script
# Note: if you need high-accuracy depth, please use `vget /camera/0/depth depth.exr`
import json; camera_trajectory = json.load(open('camera_traj.json'))
from unrealcv import client
client.connect()
# Get object information
obj_info = client.request('vget /objects')
for [loc, rot] in camera_trajectory:
    # Set position of the first camera
    client.request('vset /camera/0/location {x} {y} {z}'.format(**loc))
    client.request('vset /camera/0/rotation {pitch} {yaw} {roll}'.format(**rot))
    # Get image and ground truth
    modes = ['lit', 'object_mask']
    [im, dep, obj] = [client.request('vget /camera/0/%s' % m) for m in modes]
    print ['%s is saved to %s' % (k, v) for (k,v) in zip(modes, [im, dep, obj])]

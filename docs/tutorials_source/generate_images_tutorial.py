# Need at least 20 # characters
"""
===============
Generate Images
===============

This ipython notebook demonstrates how to generate an image dataset with rich
ground truth from a virtual environment.
"""

####################
# Load some python libraries
# The dependencies for this tutorials are
# PIL, Numpy, Matplotlib
from __future__ import division, absolute_import, print_function
import os, sys, time, re, json
import numpy as np
import matplotlib.pyplot as plt

imread = plt.imread
def imread8(im_file):
    ''' Read image as a 8-bit numpy array '''
    im = np.asarray(Image.open(im_file))
    return im

def read_png(res):
    import StringIO, PIL.Image
    img = PIL.Image.open(StringIO.StringIO(res))
    return np.asarray(img)

def read_npy(res):
    import StringIO
    return np.load(StringIO.StringIO(res))

###############################
# Connect to the game
# ===================
# Load unrealcv python client, do :code:`pip install unrealcv` first.
from unrealcv import client
client.connect()
if not client.isconnected():
    print('UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.')

###############################
# Make sure the connection works well
res = client.request('vget /unrealcv/status')
print(res)

##############################
# Load a camera trajectory
# ========================
traj_file = './camera_traj.json' # Relative to this python script
import json; camera_trajectory = json.load(open(traj_file))
# We will show how to record a camera trajectory in another tutorial

##############################
# Render an image
# ===============
idx = 1
loc, rot = camera_trajectory[idx]
# Set position of the first camera
client.request('vset /camera/0/location {x} {y} {z}'.format(**loc))
client.request('vset /camera/0/rotation {pitch} {yaw} {roll}'.format(**rot))

# Get image
res = client.request('vget /camera/0/lit lit.png')
print('The image is saved to %s' % res)

# It is also possible to get the png directly without saving to a file
res = client.request('vget /camera/0/lit png')
im = read_png(res)
print(im.shape)

# Visualize the image we just captured
plt.imshow(im)

# Ground truth generation
# =======================
# Capture an image

####################
# It is also easy to save the image to a file
res = client.request('vget /camera/0/lit output.png')
print('The file is saved to %s' % res)


####################
# Generate ground truth from this virtual scene
res = client.request('vget /camera/0/depth png')
depth = read_png(res)
res = client.request('vget /camera/0/object_mask png')
object_mask = read_png(res)
res = client.request('vget /camera/0/normal png')
normal = read_png(res)

# =================
# Visualize the captured ground truth
plt.subplot(131); plt.imshow(depth)
plt.subplot(132); plt.imshow(object_mask)
plt.subplot(133); plt.imshow(normal)

####################
# List all the objects appeared in this frame
scene_objects = client.request('vget /objects').split(' ')
print('There are %d objects in this scene' % len(scene_objects))

# TODO: replace this with a better implementation
class Color(object):
    ''' A utility class to parse color value '''
    regexp = re.compile('\(R=(.*),G=(.*),B=(.*),A=(.*)\)')
    def __init__(self, color_str):
        self.color_str = color_str
        match = self.regexp.match(color_str)
        (self.R, self.G, self.B, self.A) = [int(match.group(i)) for i in range(1,5)]

    def __repr__(self):
        return self.color_str

color_mapping = {}
inverse_color_mapping = {}
num_objects = len(scene_objects)
for idx in range(num_objects):
    objname = scene_objects[idx]
    color = Color(client.request('vget /object/%s/color' % objname))
    idx = color.R * 256 * 256 + color.G * 256 + color.B
    color_mapping[objname] = idx
    inverse_color_mapping[idx] = objname

    if idx % (num_objects / 10) == 0:
        sys.stdout.write('.')
        sys.stdout.flush()

####################
# How many objects in this frame
mask = object_mask
mask_idx = mask[:,:,0] * 256 * 256 + mask[:,:,1] * 256 + mask[:,:,2]

unique_idx = list(set(mask_idx.flatten()))
print('There are %d objects in this image' % len(unique_idx))

obj_names = [inverse_color_mapping.get(k) for k in unique_idx]
print(obj_names)


####################
# Show info of an object
# ======================
# Print an object
obj_idx = 0
obj_name = obj_names[obj_idx]
print('Show the object mask of %s' % obj_name)
mask = (mask_idx == unique_idx[obj_idx])
plt.imshow(mask)

####################
# Clean up resources
# ==================
client.disconnect()

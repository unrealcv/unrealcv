

.. _sphx_glr_tutorials_generate_images_tutorial.py:


===============
Generate Images
===============

This ipython notebook demonstrates how to generate an image dataset with rich
ground truth from a virtual environment.


Load some python libraries
The dependencies for this tutorials are
PIL, Numpy, Matplotlib



.. code-block:: python

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







Connect to the game
===================
Load unrealcv python client, do :code:`pip install unrealcv` first.



.. code-block:: python

    from unrealcv import client
    client.connect()
    if not client.isconnected():
        print('UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.')







Make sure the connection works well



.. code-block:: python

    res = client.request('vget /unrealcv/status')
    print(res)





.. rst-class:: sphx-glr-script-out

 Out::

    Is Listening
    Client Connected
    9000
    Configuration
    Config file: /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/unrealcv.ini
    Port: 9000
    Width: 640
    Height: 480


Load a camera trajectory
========================



.. code-block:: python

    traj_file = './camera_traj.json' # Relative to this python script
    import json; camera_trajectory = json.load(open(traj_file))
    # We will show how to record a camera trajectory in another tutorial







Render an image
===============



.. code-block:: python

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





.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_001.png
    :align: center


.. rst-class:: sphx-glr-script-out

 Out::

    The image is saved to /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/lit.png
    (480, 640, 4)


Ground truth generation
=======================
Generate ground truth from this virtual scene



.. code-block:: python

    res = client.request('vget /camera/0/object_mask png')
    object_mask = read_png(res)
    res = client.request('vget /camera/0/normal png')
    normal = read_png(res)

    # Visualize the captured ground truth
    plt.imshow(object_mask)
    plt.figure()
    plt.imshow(normal)




.. rst-class:: sphx-glr-horizontal


    *

      .. image:: /tutorials/images/sphx_glr_generate_images_tutorial_002.png
            :scale: 47

    *

      .. image:: /tutorials/images/sphx_glr_generate_images_tutorial_003.png
            :scale: 47




Depth is retrieved as a numpy array
For UnrealCV < v0.3.8, the depth is saved as an exr file, but this has two issues. 1. Exr is not well supported in Linux 2. It depends on OpenCV to read exr file, which is hard to install



.. code-block:: python

    res = client.request('vget /camera/0/depth npy')
    depth = read_npy(res)
    plt.imshow(depth)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_004.png
    :align: center




Get object information
======================
List all the objects appeared in the virtual scene



.. code-block:: python

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





.. rst-class:: sphx-glr-script-out

 Out::

    There are 296 objects in this scene
    .


How many objects in this frame



.. code-block:: python

    mask = object_mask
    mask_idx = mask[:,:,0] * 256 * 256 + mask[:,:,1] * 256 + mask[:,:,2]

    unique_idx = list(set(mask_idx.flatten()))
    print('There are %d objects in this image' % len(unique_idx))

    obj_names = [inverse_color_mapping.get(k) for k in unique_idx]
    print(obj_names)






.. rst-class:: sphx-glr-script-out

 Out::

    There are 48 objects in this image
    [None, 'Mug_30', 'Carpet_5', 'BookLP_142', None, 'BookLP_140', 'Couch_13', None, 'SM_Shelving_10', 'BookLP_141', None, None, 'SM_Railing_35', 'BookLP_176', None, 'SM_Railing_33', 'Switch_2', 'BookLP_104', 'SM_CoffeeTable_14', None, 'SM_Railing_34', None, 'SM_Shelving_9', None, None, 'SM_Shelving_8', None, None, None, None, None, None, None, None, None, 'BookLP_108', 'BookLP_106', 'EditorPlane_34', 'BookLP_144', None, 'SM_Room_7', None, 'BookLP_105', 'EditorPlane_24', None, 'EditorPlane_31', None, 'EditorPlane_25']


Show info of an object
======================
Print an object



.. code-block:: python

    obj_idx = 0
    obj_name = obj_names[obj_idx]
    print('Show the object mask of %s' % obj_name)
    mask = (mask_idx == unique_idx[obj_idx])
    plt.imshow(mask)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_005.png
    :align: center


.. rst-class:: sphx-glr-script-out

 Out::

    Show the object mask of None


Clean up resources
==================



.. code-block:: python

    client.disconnect()






**Total running time of the script:** ( 0 minutes  11.202 seconds)



.. container:: sphx-glr-footer


  .. container:: sphx-glr-download

     :download:`Download Python source code: generate_images_tutorial.py <generate_images_tutorial.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: generate_images_tutorial.ipynb <generate_images_tutorial.ipynb>`

.. rst-class:: sphx-glr-signature

    `Generated by Sphinx-Gallery <http://sphinx-gallery.readthedocs.io>`_

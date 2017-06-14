

.. _sphx_glr_tutorials_generate_images_tutorial.py:


===============
Generate Images
===============

This ipython notebook demonstrates how to generate an image dataset with rich
ground truth from a virtual environment.



.. code-block:: python

    import time; print(time.strftime("The last update of this file: %Y-%m-%d %H:%M:%S", time.gmtime()))





.. rst-class:: sphx-glr-script-out

 Out::

    The last update of this file: 2017-06-14 01:53:46


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
        sys.exit(-1)







Make sure the connection works well



.. code-block:: python

    res = client.request('vget /unrealcv/status')
    # The image resolution and port is configured in the config file.
    print(res)





.. rst-class:: sphx-glr-script-out

 Out::

    Is Listening
    Client Connected
    9000
    Configuration
    Config file: C:/Program Files/Epic Games/UE_4.14/Engine/Binaries/Win64/unrealcv.ini
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

    The image is saved to C:/Program Files/Epic Games/UE_4.14/Engine/Binaries/Win64/lit.png
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
List all the objects of this virtual scene



.. code-block:: python

    scene_objects = client.request('vget /objects').split(' ')
    print('Number of objects in this scene:', len(scene_objects))

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

    id2color = {} # Map from object id to the labeling color
    for obj_id in scene_objects:
        color = Color(client.request('vget /object/%s/color' % obj_id))
        id2color[obj_id] = color
        # print('%s : %s' % (obj_id, str(color)))





.. rst-class:: sphx-glr-script-out

 Out::

    Number of objects in this scene: 299


Parse the segmentation mask



.. code-block:: python

    def match_color(object_mask, target_color, tolerance=3):
        match_region = np.ones(object_mask.shape[0:2], dtype=bool)
        for c in range(3): # r,g,b
            min_val = target_color[c] - tolerance
            max_val = target_color[c] + tolerance
            channel_region = (object_mask[:,:,c] >= min_val) & (object_mask[:,:,c] <= max_val)
            match_region &= channel_region

        if match_region.sum() != 0:
            return match_region
        else:
            return None

    id2mask = {}
    for obj_id in scene_objects:
        color = id2color[obj_id]
        mask = match_color(object_mask, [color.R, color.G, color.B], tolerance = 3)
        if mask is not None:
            id2mask[obj_id] = mask
    # This may take a while
    # TODO: Need to find a faster implementation for this







Print statistics of this virtual scene and this image
=====================================================
Load information of this scene



.. code-block:: python

    with open('object_category.json') as f:
        id2category = json.load(f)
    categories = set(id2category.values())
    # Show statistics of this frame
    image_objects = id2mask.keys()
    print('Number of objects in this image:', len(image_objects))
    print('%20s : %s' % ('Category name', 'Object name'))
    for category in categories:
        objects = [v for v in image_objects if id2category.get(v) == category]
        if len(objects) > 6: # Trim the list if too long
            objects[6:] = ['...']
        if len(objects) != 0:
            print('%20s : %s' % (category, objects))





.. rst-class:: sphx-glr-script-out

 Out::

    Number of objects in this image: 118
           Category name : Object name
                Shelving : ['SM_Shelving_7', 'SM_Shelving_6', 'SM_Shelving_9', 'SM_Shelving_8']
                    Bowl : ['SM_Bowl_29']
                   Couch : ['SM_Couch_1seat_5', 'Couch_13']
                    Book : ['BookLP_139', 'BookLP_134', 'BookLP_136', 'BookLP_137', 'BookLP_130', 'BookLP_131', '...']
                DeskLamp : ['SM_DeskLamp_5']
         CoatHookBacking : ['CoatHookBacking_7']
                   Plant : ['SM_Plant_8']
                    Door : ['SM_Door_39']
              Trim_Floor : ['S_Trim_Floor_10']
                    Vase : ['SM_Vase_22', 'SM_Vase_21', 'SM_Vase_20', 'SM_Vase_18', 'SM_Vase_16', 'SM_Vase_17']
                  Carpet : ['Carpet_5', 'Carpet_7']
                    Room : ['SM_Room_7']
               FloorLamp : ['SM_FloorLamp_7']
                  Switch : ['Switch_7']
             EditorPlane : ['EditorPlane_27']
                   Frame : ['SM_Frame_39']
               WallPiece : ['WallPiece6_32', 'WallPiece2_24', 'WallPiece1_22', 'WallPiece3_26']
                CoatHook : ['CoatHook_17', 'CoatHook_16']
       RoundCeilingLight : ['SM_RoundCeilingLight_4']
             CoffeeTable : ['SM_CoffeeTable_14']


Show the annotation color of some objects



.. code-block:: python

    ids = ['SM_Couch_1seat_5', 'SM_Vase_17', 'SM_Shelving_6', 'SM_Plant_8']
    # for obj_id in ids:
    obj_id = ids[0]
    color = id2color[obj_id]
    print('%s : %s' % (obj_id, str(color)))
    # color_block = np.zeros((100,100, 3)) + np.array([color.R, color.G, color.B]) / 255.0
    # plt.figure(); plt.imshow(color_block); plt.title(obj_id)





.. rst-class:: sphx-glr-script-out

 Out::

    SM_Couch_1seat_5 : (R=255,G=0,B=255,A=255)


Plot only one object



.. code-block:: python

    mask = id2mask['SM_Plant_8']
    plt.figure(); plt.imshow(mask)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_005.png
    :align: center




Show all sofas in this image



.. code-block:: python

    couch_instance = [v for v in image_objects if id2category.get(v) == 'Couch']
    mask = sum(id2mask[v] for v in couch_instance)
    plt.figure(); plt.imshow(mask)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_006.png
    :align: center




Change the annotation color, fixed in v0.3.9
You can use this to make objects you don't care the same color



.. code-block:: python

    client.request('vset /object/SM_Couch_1seat_5/color 255 0 0') # Change to pure red
    client.request('vget /object/SM_Couch_1seat_5/color')
    res = client.request('vget /camera/0/object_mask png')
    object_mask = read_png(res)
    plt.imshow(object_mask)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_007.png
    :align: center




Clean up resources
==================



.. code-block:: python

    client.disconnect()






**Total running time of the script:** ( 0 minutes  6.542 seconds)



.. container:: sphx-glr-footer


  .. container:: sphx-glr-download

     :download:`Download Python source code: generate_images_tutorial.py <generate_images_tutorial.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: generate_images_tutorial.ipynb <generate_images_tutorial.ipynb>`

.. rst-class:: sphx-glr-signature

    `Generated by Sphinx-Gallery <https://sphinx-gallery.readthedocs.io>`_

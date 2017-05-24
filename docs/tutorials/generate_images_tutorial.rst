

.. _sphx_glr_tutorials_generate_images_tutorial.py:


===============
Generate Images
===============

This ipython notebook demonstrates how to generate an image dataset with rich
ground truth from a virtual environment.


Load some python libraries



.. code-block:: python

    from __future__ import division, absolute_import, print_function
    import os, sys, time, re, json
    import numpy as np
    # Load matplotlib for plotting
    import matplotlib.pyplot as plt
    imread = plt.imread
    from PIL import Image # Use pillow

    def imread8(im_file):
        # Read image as a 8-bit numpy array
        im = np.asarray(Image.open(im_file))
        return im







Start the docker image
======================



.. code-block:: python

    from docker_util import runner
    runner.start()





.. rst-class:: sphx-glr-script-out

 Out::

    Can not find images, downloading
    It is equivalent to run: nvidia-docker run -d -it --env="DISPLAY=:0.0" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" -p 9000:9000 -v /home/qiuwch/workspace/unrealcv-develop-branch/docs/tutorials_source/output:/home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output qiuwch/rr /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering


Connect to the game
===================
Load unrealcv python client, do :code:`pip install unrealcv` first.



.. code-block:: python

    from unrealcv import client
    client.connect()
    if not client.isconnected():
        print('UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.')
    # Make sure the connection works well
    res = client.request('vget /unrealcv/status')
    print(res)





.. rst-class:: sphx-glr-script-out

 Out::

    Is Listening
    Client Connected
    9000


Load a camera trajectory



.. code-block:: python

    import json; camera_trajectory = json.load(open('camera_traj.json'))








Move the camera to a new location and capture an image



.. code-block:: python

    frames = []
    N = len(camera_trajectory)
    N = 1
    for idx in range(N):
        loc, rot = camera_trajectory[idx]
        # Set position of the first camera
        client.request('vset /camera/0/location {x} {y} {z}'.format(**loc))
        client.request('vset /camera/0/rotation {pitch} {yaw} {roll}'.format(**rot))
        # Get image and ground truth
        modes = ['lit', 'depth', 'object_mask']
        frame = dict()
        for m in modes:
            res = client.request('vget /camera/0/{type} output/{idx}_{type}.png'.format(type = m, idx=idx))
            print(res)
            # TODO: this is a temporary hack, fix it later
            res = os.path.join('output', os.path.basename(res))
            msg = '%s is saved to %s' % (m, res)
            frame[m] = res

        frames.append(frame)





.. rst-class:: sphx-glr-script-out

 Out::

    /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output/0_lit.png
    /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output/0_depth.png
    /home/unrealcv/LinuxNoEditor/RealisticRendering/Binaries/Linux/output/0_object_mask.png


Visualize a frame
=================



.. code-block:: python

    def plot_frame(frame):
        def subplot(sub_index, image, param=None):
            if isinstance(image, str):
                image = imread(image)
            plt.subplot(sub_index)
            plt.imshow(image, param)
            plt.axis('off')

        subplot(131, frame['lit'])
        subplot(132, frame['depth'])
        subplot(133, frame['object_mask'])
        plt.tight_layout()

    plot_frame(frames[0])




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_001.png
    :align: center




List all the objects appeared in this frame



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

    There are 295 objects in this scene
    ....


How many objects in this frame



.. code-block:: python

    frame = frames[0]
    mask_file = frame['object_mask']
    mask = imread8(mask_file)
    mask_idx = mask[:,:,0] * 256 * 256 + mask[:,:,1] * 256 + mask[:,:,2]

    unique_idx = list(set(mask_idx.flatten()))
    print('There are %d objects in this image' % len(unique_idx))

    obj_names = [inverse_color_mapping.get(k) for k in unique_idx]
    print(obj_names)






.. rst-class:: sphx-glr-script-out

 Out::

    There are 14 objects in this image
    ['Couch_13', 'Carpet_5', 'BookLP_176', 'EditorPlane_34', 'SM_Shelving_10', 'EditorPlane_31', 'SM_CoffeeTable_14', None, None, 'EditorPlane_25', None, None, 'Mug_30', None]


Show info of an object
======================
Print an object



.. code-block:: python

    obj_idx = 0
    obj_name = obj_names[obj_idx]
    print('Show the object mask of %s' % obj_name)
    mask = (mask_idx == unique_idx[obj_idx])
    plt.imshow(mask)




.. image:: /tutorials/images/sphx_glr_generate_images_tutorial_002.png
    :align: center


.. rst-class:: sphx-glr-script-out

 Out::

    Show the object mask of Couch_13


Clean up resources
==================
Stop docker image



.. code-block:: python

    runner.stop()
    client.disconnect()






**Total running time of the script:** ( 0 minutes  12.372 seconds)



.. container:: sphx-glr-footer


  .. container:: sphx-glr-download

     :download:`Download Python source code: generate_images_tutorial.py <generate_images_tutorial.py>`



  .. container:: sphx-glr-download

     :download:`Download Jupyter notebook: generate_images_tutorial.ipynb <generate_images_tutorial.ipynb>`

.. rst-class:: sphx-glr-signature

    `Generated by Sphinx-Gallery <http://sphinx-gallery.readthedocs.io>`_

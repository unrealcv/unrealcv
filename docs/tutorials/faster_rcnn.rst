==============================
Integration with an AI program
==============================

This is a video showing connecting Faster-RCNN with a video game for diagnosis.

.. raw:: html

    <iframe width="560" height="315" src="https://www.youtube.com/embed/iGGNAkaxVyQ" frameborder="0" allowfullscreen></iframe>

.. warning::

    We are working on a python layer for Caffe and will include it in the next release.

Read :doc:`getting started </tutorials/getting_started>` first and know how to use the compiled binary. Faster-RCNN is an algorithm to detect objects in an image. The original paper is published `here <http://arxiv.org/abs/1506.01497>`__. In this tutorial we show how to run Faster-RCNN in the virtual scene. You can observe the Faster-RCNN can do a pretty good job, but still consistently making some mistakes, for example, detect photos as TV monitor.

Run the demo
============

To run the program illustrated in the video.

1. Install `py-faster-rcnn <https://github.com/rbgirshick/py-faster-rcnn>`__. Follow the instruction to setup. Make sure you can run :code:`tools/demo.py`

2. Run RealisticRendering in the :ref:`rr`

3. Modify :code:`rcnn_path` in the script :file:`client/examples/faster-rcnn.py` to point to your Faster-RCNN installation, then run it.

.. code:: bash

    cd client/examples
    python faster-rcnn.py

After running :code:`faster-rcnn.py`, you are expected to see a message showing the client successfully connected to the game. When you clicked mouse in the game, a frame will be sent to Faster RCNN for detection. Using the same technique shown in this tutorial, algorithms used for depth estimation, object recognition can be also easily integrated.

Here we show how to do testing in a virtual scene. If you are interested in training model with synthetic images. Please refer to the tutorial about :doc:`generating images </tutorials/generate_images_tutorial>`.

.. If you want the training procedure can also interact with the scene, such as reinforcement learning. please join the discussion `here <https://groups.google.com/d/topic/unrealcv/0zFjX_DCe7U/discussion>`__, we are working on it.

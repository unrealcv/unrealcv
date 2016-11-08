This is a video showing connecting Faster-RCNN with a video game for diagnosis.

<iframe width="560" height="315" src="https://www.youtube.com/embed/iGGNAkaxVyQ" frameborder="0" allowfullscreen></iframe>

<!-- ![Caffe Integration](images/caffe_integration.png) -->

<blockquote class="bg-warning">
</blockquote>

<blockquote class="bg-warning">
We are working on a python layer for Caffe and will include it in the next release.
</blockquote>

Make sure you read [Getting Started](/binary/getting-started.md) first and know how to use the compiled binary. Faster-RCNN is an algorithm to detect objects in an image. The original paper is published [here](http://arxiv.org/abs/1506.01497). In this tutorial we show how to run Faster-RCNN in the virtual scene. You can observe the Faster-RCNN can do a pretty good job, but still consistently making some mistakes, for example, detect photos as TV monitor.

## Setup


## Run demo

To run the program illustrated in the video.

1. Install [py-faster-rcnn](https://github.com/rbgirshick/py-faster-rcnn). Follow the instruction to setup. Make sure you can run `tools/demo.py`

2. Run RealisticRendering in [Getting Started](getting_started.html)

3. Modify `rcnn_path` to point to your Faster-RCNN installation. Run {% include script.html key="faster-rcnn" %}

``` bash
cd client/examples
python faster-rcnn.py
```

After running `faster-rcnn.py`, you are expected to see a message showing the client has already successfully connected to the game.

When you clicked mouse in the game, a frame will be sent to Faster RCNN for detection. Using the same technique shown in this tutorial, algorithms used for depth estimation, object recognition can be also easily integrated.

Here we show how to do testing in a virtual scene. If you are interested in training model with synthetic images. Please refer to {% include script.html key='ipynb_generate_images' %}. If you want the training procedure can also interact with the scene, such as reinforcement learning. [contact us](/contact.html).

The quantitative evaluation is not included in this tutorial.

This page introduces *UnrealCV commands* and how to use them to perform basic tasks. We also show how to use a python script to control an UnrealCV embedded game through these commands.

## 1. Download a game binary

This tutorial will use a game binary to demonstrate UnrealCV commands. You can also [create your own game using UnrealCV plugin](plugin/usage.md).

First you need to download a game binary from [our model zoo](reference/model-zoo.md). For this tutorial, please download [RealisticRendering](reference/model-zoo.md#realistic_rendering). After unzip and run the binary, you are expected to see a screen like this. The game will be started in a window mode with resolution 640x480, you can change the resolution by [changing the configuration file](game/configuration.md) of UnrealCV.

<center>
  <img alt="Startup Screenshot" src="/images/rr_init.png" width="300px"/>
  <p>Fig.1. Initial screen of the game</p>
</center>

Use mouse to look around and use keys `w a s d` to navigate, use `q e` to level the camera up and down. If you want to release mouse cursor from the game, press <code>&#96;</code> (the key on top of tab).

## 2. UnrealCV commands

UnrealCV provides a set of commands for computer vision research. These commands are used to perform various tasks, such as control camera and get ground truth. The table below summaries commands used in this tutorial. The complete list can be found in [the command list](reference/commands.html) in the reference section.

| Command                             | Help                                              |
|:------------------------------------|:--------------------------------------------------|
| vset /viewmode [viewmode_name]      | Set ViewMode to (lit, normal, depth, object_mask) |
| vget /camera/0/lit                  | Save image to disk and return filename            |
| vset /camera/0/location [x] [y] [z] | Set camera location                               |

<h3>Try UnrealCV commands</h3>
Unreal Engine provides a built-in console to help developers to debug games. This built-in console is a convenient way of trying UnrealCV commands. To open the console, press
<code>&#96;</code>
(the key on top of tab) twice, a console will pop out, shown in Fig.2. Type in `vset /viewmode object_mask` you are expected to see the object instance mask. Use `vset /viewmode lit` to switch back to normal rendering setting.

<center>
  <img alt="console" src="/images/console.png"/>
  <p>Fig.2 Use console to try UnrealCV commands</p>
</center>

## 3. Use python client to execute commands

If we want to generate a large-scale synthetic dataset, or do active tasks, such as reinforcement learning, in this virtual world. We need to allow an intelligent agent to perceive, navigate and interact in the scene. We provide UnrealCV client to enable other programs to communicate with this virtual world. The client will use a [plain-text protocol](reference/arch.md#protocol) to exchange information with the game.

Here we use the python client for illustration. If you are looking for a MATLAB client, please see [here](reference/api.md#matlab).

First, we need to install the python client library.

<h3>Install UnrealCV python library</h3>
```shell
pip install unrealcv
```

<h3>Generate some images from the scene</h3>
```python
from unrealcv import client
client.connect() # Connect to the game
if not client.isconnected(): # Check if the connection is successfully established
  print 'UnrealCV server is not running. Run the game from http://unrealcv.github.io first.'
else:
  filename = client.request('vget /camera/0/lit')
  filename = client.request('vget /camera/0/depth depth.exr')
```
You can find this example in [client/examples/10lines.py]().

If you encountered any errors following this tutorial, please see [the diagnosis]() page to find a solution.


## Next

<h3>Use UnrealCV in the game mode or plugin mode?</h3>

For the game mode, you can use a compiled game binary. You can freely control the camera in this game and generate images and ground truth from it. But it is not easy to change the scene, such as add more objects or change the material properties. If you have access to an UE4 project and know how to use the UE4Editor, you can install the plugin to UE4Editor, so that you can combine the power of UE4Editor and UnrealCV to create new virtual worlds for research.

<h3>Tutorials</h3>

- [How to generate an image dataset](game/generate-images.md)
- [Integrate with a deep learning framework](game/faster-rcnn.md)
- [Use the plugin in UE4Editor](plugin/usage.md)
- [Modify code and add a new command](plugin/add-command.md)

<h3>Articles</h3>

- To fully understand how does UnrealCV work and the technical details, please read its [architecture](/reference/architecture.html) or [our paper](http://arxiv.org/abs/1609.01326). For a complete list of available commands, please read [the command list](/reference/commands.html) in the reference section.

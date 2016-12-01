This page covers the basics of UnrealCV. It includes how to use UnrealCV commands to perform basic task and how to use a python script to control an UnrealCV embedded game.

## Game mode or plugin mode?

In the game mode, you can download a compiled binary created by the community. In this binary, you can generate images, ground truth. But it is not easy to change the scene by adding more objects.

<!-- the virtual world definition from unrealcv paper  -->

Install the plugin to UE4 editor, you can use the power of UE4 editor to modify the virtual world as you wish.

The images for these two modes are shown below.

TODO

## 1. Download a game binary

This tutorial will use a game binary to demonstrate UnrealCV commands. If you are interested in using the plugin in UE4Editor or produce binaries, please finish this tutorial, then go to [Plugin Mode - Usage](/plugin/usage.md).

A list of games can be found in [Model Zoo](TODO). For this tutorial, please download [RealisticRendering](/reference/model_zoo.html#realistic_rendering).

After unzip and run the binary, you are expected to see a screen like this. The game will be started in a window mode with resolution 640x480, you can change the resolution by changing the configuration file of UnrealCV, see [here](TODO).

<center>
  <img alt="Startup Screenshot" src="../../images/rr_init.png" width="300px"/>
  <p>Fig.1. Initial screen of the game</p>
</center>

Use mouse to look around and use keys `wasd` to navigate, use `qe` to level the camera up and down. If you want to release mouse cursor from the game, press <code>&#96;</code> (the key on top of tab).

## 2. UnrealCV commands

UnrealCV provides a set of commands for computer vision research. These commands are used to perform various tasks, such as control camera and get ground truth. The table below summaries commands used in this tutorial. The complete list can be found in [Reference: Command List](/reference/commands.html).

<!-- TODO: consider making this external -->
| Command                             | Help                                              |
|:------------------------------------|:--------------------------------------------------|
| vset /viewmode [viewmode_name]      | Set ViewMode to (lit, normal, depth, object_mask) |
| vget /camera/0/lit                  | Save image to disk and return filename            |
| vset /camera/0/location [x] [y] [z] | Set camera location                               |

### Try UnrealCV commands
Unreal Engine provides a built-in console to help developers to debug games. This built-in console is a convenient way of trying UnrealCV commands. To open the console, press
<code>&#96;</code>
(the key on top of tab) twice, a console will pop out, shown in Fig.2. Type in `vset /viewmode object_mask` you are expected to see the object instance mask. Use `vset /viewmode lit` to switch back to normal rendering setting.

<center>
  <img alt="console" src="../../images/console.png" width="600px"/>
  <p>Fig.2 Use console to try UnrealCV commands</p>
</center>


## 3. Use UnrealCV python client to execute commands

If we want to generate a large-scale synthetic dataset, or do active tasks, such as reinforcement learning, in this virtual world. We need to allow an intelligent agent to perceive, navigate and interact in the scene. We provide UnrealCV client to enable other programs to communicate with this virtual world. The client will use a plain-text protocol to exchange information with the game.

Here we use the python client for illustration. If you are looking for a MATLAB client, please see [here](/reference/client.html#matlab). First, we need to install the python client library.

### Install UnrealCV python library
```shell
pip install unrealcv
```

### Generate some images from the scene
```python
from unrealcv import client
client.connect() # Connect to the game
if not client.isconnected(): # Check if the connection is successfully established
  print 'UnrealCV server is not running. Run the game from http://unrealcv.github.io first.'
else:
  filename = client.request('vget /camera/0/lit')
  filename = client.request('vget /camera/0/depth depth.exr')
```
You can find this example in {% include script.html key='demo.py' %}.

## The speed of UnrealCV

## Next:

- To fully understand how does UnrealCV work and the technical details, please read [Reference: Architecture](/reference/architecture.html) or [our paper](http://arxiv.org/abs/1609.01326). For a complete list of available commands, please read [Reference: Commands](/reference/commands.html).

- For a more complete example of generating a dataset from the virtual world, please read {% include script.html key='ipynb_generate_images' text='Tutorial: Generate Images' %}

- If you are interested in how to integrate your external program with UnrealCV, please read [Tutorial: Integrate with Faster-RCNN](/tutorial/faster_rcnn.html).

- If you are interested in using the plugin in UE4Editor, please finish this tutorial, then go to [Tutorial: Use Plugin](/tutorial/plugin.html).

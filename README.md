# UnrealCV

[![Build Status](https://travis-ci.com/qiuwch/unrealcv.svg?token=x3MJbq7z7sZyEUorGp5T&branch=master)](https://travis-ci.com/qiuwch/unrealcv)

UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine 4 (UE4). It extends UE4 by providing:

1. Communication between UE4 and an external program, such as Caffe.
2. A set of UE4 commands to control the camera and get ground truth.

Ground truth includes depth, surface normal and object instance mask. More will be added in future release.
<!-- ![architecture](http://weichaoqiu.com:4000/images/pipeline.svg) -->
![Teaser](http://weichaoqiu.com:4000/images/teaser.png)

UnrealCV contains two parts, plugin and client. Plugin supports windows, liunx and mac. UnrealCV client is provided for Python and MATLAB.

## Use Game Binaries

Using UnrealCV, you can use Unreal Engine to create a virtual world for your research. 

We provide game binaries from which you can generate images and ground truth. These binaries can be downloaded from [here](http://www.weichaoqiu.com/unrealcv/model_zoo.html). 

### Commands

UE4 has a built-in development console, in which command can be used to perform tasks. UnrealCV provides a set of UE4 commands, these commands are designed for computer vision research. Image and ground truth are generated using commands, such as `vget /camera/0/image`. The complete list of UnrealCV commands can be found [here](http://weichaoqiu.com:4000/reference/commands.html)

| Command                             | Help                |
| ---                                 | ---                 |
| vset /viewmode [str]                | Set ViewMode to (lit, normal, depth, object_mask)       |
| vget /camera/0/lit                  | Get Image           |
| vset /camera/0/location [x] [y] [z] | Set Camera Location |


### Client

UnrealCV client is used to send commands to UnrealCV server. The python unrealcv client can be installed using `pip install unrealcv`. For MATLAB client, please see file `client/matlab/demo.m`.

```python
from unrealcv import client
client.connect() # Connect to the game
if not client.isconnected(): # Check if the connection is successfully established
    print 'UnrealCV server is not running. Run the game from http://unrealcv.github.io first.'
else:
    filename = client.request('vget /camera/0/lit')
    print 'Image is saved to %s' % filename
    for gt_type in ['depth', 'normal', 'object_mask']:
        filename = client.request('vget /camera/0/%s' % gt_type) 
        print '%s is saved to %s' % (gt_type, filename)
```

An [IPython Notebook](http://www.weichaoqiu.com/unrealcv/ipynb_generate_images.html) tutorial provides step-by-step instruction to generate an image dataset from the binary.

## Install UnrealCV to UE4 Editor

UnrealCV provides a plugin for Unreal Engine. After installing the plugin, you can use the game design tools of UE4 to create a virtual world and let computer vision algorithms to interact with this virtual worlds.

If needs to modify the objects of the scene, such as changing object material, lighting configuration. It is easier to use UnrealEditor.

### Prerequisites

- Install and get familiar with Unreal Engine Editor (UEditor), Installation guide [Windows and Mac](https://docs.unrealengine.com/latest/INT/GettingStarted/Installation/). [Linux](https://wiki.unrealengine.com/Building_On_Linux)
- Learn how to use UEditor

### Installation

UnrealCV plugin needs to be downloaded and copied to the `Plugins` folder. We provide a script to simplify the installation.
UnrealCV can be installed to a project or to the engine folder.

- Install to project
    - go to project folder which contains `[ProjectName].uproject`
    - create a folder called `Plugins`
    - run `curl weichaoqiu.com:4000/install.sh | sh`

- Install to Unreal Engine
    - go to the plugin folder of Unreal Engine which is `Engine/Plugins`
    - run `curl weichaoqiu.com:4000/install.sh | sh`

- Open `Menu -> Edit -> Plugins`, make sure `UnrealCV` is installed and enabled.
![instal-plugin](http://weichaoqiu.com:4000/images/install-plugin.png)

```
- Plugins
  - UnrealCV
    - unrealcv.uplugin
```

### Usage

In `World Settings -> Game Mode -> GameMode Override`, select `UE4CVGameMode`. Then you can use the functions of UnrealCV in the UEditor.

# Citation

If you found this project useful, please consider citing our paper

'''

'''

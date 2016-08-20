# UnrealCV

[![Build Status](https://travis-ci.com/qiuwch/unrealcv.svg?token=x3MJbq7z7sZyEUorGp5T&branch=master)](https://travis-ci.com/qiuwch/unrealcv). 

UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine 4 (UE4). UnrealCV provides:

1. Communication
2. CV related functions

Images and ground truth can be easily generated from a game. Ground truth includes depth, surface normal and object instance mask. More ground truth will be added in future release.
<!-- ![architecture](http://weichaoqiu.com:4000/images/pipeline.svg) -->
![Teaser](http://weichaoqiu.com:4000/images/teaser.png)

# Use Binaries

We provide game binaries from which you can generate images and ground truth. These binaries can be downloaded from [here](http://www.weichaoqiu.com/unrealcv/model_zoo.html). An [IPython Notebook](http://www.weichaoqiu.com/unrealcv/ipynb_generate_images.html) tutorial provides step-by-step instruction.

Image and ground truth are generated using commands, such as `vget /camera/0/image`.

UnrealCV client.
```python
import ue4cv
ue4cv.client.connect() # Connect to the game
if not ue4cv.client.isconnected(): # Check if the connection is successfully established
    print 'UnrealCV server is not running. Run the game from http://unrealcv.github.io first.'
else:
    filename = ue4cv.client.request('vget /camera/0/lit')
    print 'Image is saved to %s' % filename
    for gt_type in ['depth', 'normal', 'object_mask']:
        filename = ue4cv.client.request('vget /camera/0/%s' % gt_type) 
        print '%s is saved to %s' % (gt_type, filename)
```

# Use UnrealCV in UE4 Editor

UnrealCV provides a plugin for Unreal Engine. After installing the plugin, you can use the game design tools of UE4 to create a virtual world and let computer vision algorithms to interact with this virtual worlds.

If needs to modify the objects of the scene, such as changing object material, lighting configuration. It is easier to use UnrealEditor. UnrealCV can work well  with UnrealEditor.

## Installation
Install to project or install to engine.

Download and copy the plugin to project folder. In the _*.uproject_. The file structure will look like
```
- [ProjectName].uproject
- Plugins
  - UnrealCV
    - unrealcv.uplugin
```

## After installation, you can change the GameMode to capture

# Citation

If you found this project useful, please consider citing our paper

'''

'''

# UnrealCV

[![Build Status](https://travis-ci.com/qiuwch/unrealcv.svg?token=x3MJbq7z7sZyEUorGp5T&branch=master)](https://travis-ci.com/qiuwch/unrealcv)

UnrealCV is a project to help computer vision researchers use Unreal Engine 4 (UE4). UnrealCV allows generating images and ground truth from a game easily.

<!-- ![architecture](http://weichaoqiu.com:4000/images/pipeline.svg) -->
![Teaser](http://weichaoqiu.com:4000/images/Annotation.png)

# Use standalone binaries

If you don't have any experience with Unreal Engine, the simplest way of getting started is downloading binaries we created. Synthetic images and ground truth can be generated from these binaries using UnrealCV. A [ipython notebook](http://www.weichaoqiu.com/unrealcv/ipynb_generate_images.html) tutorial provides step-by-step instruction.

Game binaries can be found [here](http://www.weichaoqiu.com/unrealcv/model_zoo.html).


# Use UnrealCV in UE4 Editor

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

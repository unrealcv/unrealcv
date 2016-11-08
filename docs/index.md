## Introduction
UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine 4 (UE4). It extends UE4 with a plugin by providing:

1. A set of UnrealCV commands to interact with the virtual world.
2. Communication between UE4 and an external program, such as Caffe.

<center>
<img src="images/homepage_teaser.png" alt="annotation"/>
Images generated from the technical demo <a href="reference/model_zoo.html#realistic_rendering">RelisticRendering</a><br>
</center>

UnrealCV can be used in two ways. The first is using a compiled game binary with UnrealCV embedded. This is as simple as running a game, no knowledge of Unreal Engine is required. The second is installing UnrealCV plugin to Unreal Engine 4 (UE4) and use the editor of UE4 to build a new virtual world.

Before reading this documentation, please read [Getting Started](/binary/getting-started.md) to help you familiarize what UnrealCV is about. This goal of this documentation is helping you integrate UnrealCV with your research project.


## Outline
This documentation is organized as follow:

- "Use game binaries" teaches how to use a compiled game binary with UnrealCV embedded
    - Model Zoo
- "Use UnrealCV Plugin" provides information about using UnrealCV editor plugin
    - [Installation](plugin/install.md)
- "Development" provides information for modifying UnrealCV source code
    - [Environment setup](dev/setup.md)
    - [Modify UnrealCV code to add more functionalities](dev/example-add-new-command.md)
- "Reference" is an index of resources
    - [Architecture](ref/arch.md)
    - [UnrealCV commands](ref/command.md)

## Contact

This documentation is written with markdown, hosted in ReadTheDocs. All the markdown source files are hosted [here](https://github.com/unrealcv/unrealcv/tree/master/docs). To see the docs for different versions, please check this [versions](https://readthedocs.org/projects/unrealcv/versions/) page.

If you found any part of this documentation is unclear, please feel free to ask question in our gitter channel. You are welcome to contribute by making a pull request or report issues in [issue tracker](https://github.com/unrealcv/unrealcv/issues).

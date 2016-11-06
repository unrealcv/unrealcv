UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine 4 (UE4). It extends UE4 with a plugin by providing:

1. A set of UnrealCV commands to interact with the virtual world.
2. Communication between UE4 and an external program, such as Caffe.

UnrealCV can be used in two ways. The first one is using a compiled game binary with UnrealCV embedded. This is as simple as running a game, no knowledge of Unreal Engine is required. The second is installing UnrealCV plugin to Unreal Engine 4 (UE4) and use the editor of UE4 to build a new virtual world.

Before reading this documentation, please read [Tutorial - Getting Started](https://unrealcv.github.io/tutorial/getting_started.html) to help you familiarize what UnrealCV is about. The goal of UnrealCV documentation is helping you integrate UnrealCV with your research project,

## Outline
This documentation covers following topics:

- [Getting started]()  - Basic knowledge of using UnrealCV
- Use game binaries
    - Model Zoo
- Use UnrealCV Plugin
    - [Installation](plugin/install.md)
- Development
    - [Environment setup](dev/setup.md)
    - [Modify UnrealCV code to add more functionalities](dev/example-add-new-command.md)
- Reference
    - [Architecture](ref/arch.md)
    - [UnrealCV commands](ref/command.md)

How to use UnrealCV plugin is described in this page.

## Contact

This documentation is written with markdown, hosted in ReadTheDocs. All the markdown source files are hosted [here](https://github.com/unrealcv/unrealcv/tree/master/docs). To see the docs for different versions, please check this [versions](https://readthedocs.org/projects/unrealcv/versions/) page.

You are welcome to contribute by making a pull request or report issues in [issue tracker](https://github.com/unrealcv/unrealcv/issues).

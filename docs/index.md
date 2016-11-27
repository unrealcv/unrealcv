## Introduction
UnrealCV is a project to help computer vision researchers build virtual worlds using Unreal Engine 4 (UE4). It extends UE4 with a plugin by providing:

1. A set of UnrealCV commands to interact with the virtual world.
2. Communication between UE4 and an external program, such as Caffe.

![teaser](images/homepage_teaser.png)
Images generated from the technical demo [RealisticRendering](reference/model_zoo.html#realistic_rendering)

The goal of this documentation is to help you integrate UnrealCV with your research project.


## Documentation Outline

UnrealCV is an UE4 plugin, but it can be used in two modes and this documentation will show which mode is suitable for you and how to use it in these two modes.

1. `Game` mode: use precompiled binaries provided by Unrealcv community. This is as simple as running a video game, no knowledge of Unreal Engine is required.    
2. `Plugin` mode: use unrealcv plugin in UE4Editor. In this mode, UnrealCV is installed into UE4 as an editor plugin and can be used to build a virtual world.

Unrealcv consists two parts:   
1. `unrealcv server` which is embedded into a video game (in game mode)     
or in the UE4 editor (in plugin mode).      
2. `unrealcv client` use commands to control the server.

This documentation is organized as follow:

- UnrealCV basics
    - [Getting Started](basic/getting-started.md)
        - Download and install
        - [Game mode or plugin mode](basic/game_or_plugin.md)
        - UnrealCV Commands

- Game Mode
    - [Usage](game/usage.md)
    - [Tutorial: Generate images](game/generate.md)
    - [Tutorial: Integrate with other programs]()

- Plugin Mode
    - [Usage](plugin/usage.md)
        - [Install](plugin/usage.md#install)
    - [Development](plugin/dev.md)
        - [Environment setup](dev/setup.md)
        - [Modify UnrealCV code to add more functionalities](dev/example-add-new-command.md)

- Reference
    - Model Zoo
    - [Architecture](reference/arch.md)
    - [Command list](reference/command.md)


## Contact

This documentation is written with markdown, hosted in ReadTheDocs. All the markdown source files are hosted [here](https://github.com/unrealcv/unrealcv/tree/master/docs). To see the docs for different versions, please check this [versions](https://readthedocs.org/projects/unrealcv/versions/) page.

If you found any part of this documentation is unclear, please feel free to ask question in our gitter channel. You are welcome to contribute by making a pull request or report issues in [issue tracker](https://github.com/unrealcv/unrealcv/issues).

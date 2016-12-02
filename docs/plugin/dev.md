## Setting up development environment
UnrealCV can be compiled seperately as a plugin as shown in [here](plugin.md#compilation)
, or be compiled together with an UE4 C++ project. If you just want to produce binaries from a newer version of code. Simply type `make` can produce binary for you.

## Blueprint project or C++ project

We suggest developing UnrealCV together with a C++ Unreal Engine project. You can develop UnrealCV with any UE4 C++ project, if the project you are working on is not a C++ project, you can add a C++ class to make it a C++ project.

The simplest way is starting from the [playground project](https://github.com/unrealcv/playground). Playground project is a simple UE4 project to show the features or UE4 and UnrealCV, it serves as a development base and test platform for unrealcv team.

The playground project can be got by
```
git clone https://github.com/unrealcv/playground.git
```

- Windows

    To generate visual studio solution, right click `playground.uproject` and choose `Generate Visual Studio project files`.

    The plugin code will be compiled together with the game project.

    ---

- Linux

    ```
    Under construction
    ```

    Generate project file and use Makefile

    ---

- Mac

    Ask @siyuan how to compile in Mac.

---
## Code Documentation

The C++ source code of the plugin is documented with doxygen. You can generate the document for code by running `cd docs/doxygen; doxygen`. An online version is hosted [here](https://codedocs.xyz/unrealcv/unrealcv/).

---
## UnrealCV Architecture

The initialization procedure of UnrealCV. The module loaded and start a TCP server waiting for connection. Commands will be parsed by regexp.

---
The code is here {{ config.extra.codebas }}

## Project Layout
    client/            # Client code for Python and MATLAB
        examples/      # Examples showing how to use client code to do tasks
        matlab/
        python/
        scripts/       # Scripts for automatic tasks, such as Jenkins build
    Content/           # Plugin data
    docs/              # Documentation of UnrealCV
    Resources/         # Plugin resource
    Source/            # Plugin C++ source code
    test/              # Test code
    UnrealCV.uplugin   #
    README.md

---

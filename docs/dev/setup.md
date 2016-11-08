This page will cover how to setup dev environment.

## Compilation
UnrealCV can be compiled seperately as a plugin as shown in [here](plugin.md#compilation)
, or be compiled together with an UE4 C++ project.

If you just want to produce binaries from a newer version of code. Simply type `make` can produce binary for you.

## Setting up development environment
We suggest developing UnrealCV together with a C++ Unreal Engine project. You can develop UnrealCV with any UE4 C++ project, if the project you are working on is not a C++ project, you can add a C++ class to make it a C++ project.

### Windows

The simplest way is starting from the [playground project](https://github.com/unrealcv/playground).

This project can be got by
```
git clone https://github.com/unrealcv/playground.git
```

Make sure everything is working well by running
```
python test/
```

To generate visual studio solution, right click `playground.uproject` and choose "Generate Visual Studio project files".

The plugin code will be compiled together with the game project.

### Linux (coming)

```
Under construction
```
Generate project file and use Makefile

## Code documentation

The C++ source code of the plugin is documented with doxygen.

You can generate the document for code by running `cd docs/doxygen; doxygen`.

## UnrealCV basics

The initialization procedure of UnrealCV. The module loaded and start a TCP server waiting for connection. Commands will be parsed by regexp. 

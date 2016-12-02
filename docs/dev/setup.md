    Topics will be covered in this page:   
    - Setup dev environment
    -

## Compilation
UnrealCV can be compiled seperately as a plugin as shown in [here](plugin.md#compilation)
, or be compiled together with an UE4 C++ project.

If you just want to produce binaries from a newer version of code. Simply type `make` can produce binary for you.

## Setting up development environment
We suggest developing UnrealCV together with a C++ Unreal Engine project. In this way,


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

Compilation.

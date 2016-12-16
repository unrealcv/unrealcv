# Setting up development environment
UnrealCV can be compiled as a plugin as shown in [the plugin install instruction](/plugin/usage.md), or be compiled together with an UE4 C++ project.

## 1. Create a C++ game project

UE4 has two types of game projects. Blueprint project and C++ project, in a blueprint project, the game logic is designed by blueprint scripting languages, a specific language created by Unreal Engine team. These two types of projects are easily convertable.

In a C++ project, the plugin code will be compiled together with the game project. We suggest developing UnrealCV together with a C++ Unreal Engine project.

The simplest way to start is using the [playground project](https://github.com/unrealcv/playground). Playground project is a simple UE4 project to show the features or UE4 and UnrealCV, it serves as a development base and test platform for UnrealCV team.

The playground project is hosted [here](github.com/unrealcv/playground.git) in github. You can get it by

```
git clone --recursive https://github.com/unrealcv/playground.git
```

UnrealCV is a submodule of this repository. If you cloned the project without `--recursive` and found the folder `Plugins/unrealcv` empty. You can use `git submodule init; git submodule update` to get the UnrealCV code.

## 2. Compile the C++ project

### Windows

Install visual studio.

To generate visual studio solution, right click `playground.uproject` and choose `Generate Visual Studio project files`.

The plugin code will be compiled together with the game project.

### Linux

- Compile UE4 from source code following [this instruction](https://wiki.unrealengine.com/Building_On_Linux)

- Put this to your `.bashrc` or `.zshrc`

```bash
# Modify to a path that specified in step 1
export UE4=/home/qiuwch/UnrealEngine/4.13
UE4Editor() { bin=${UE4}/Engine/Binaries/Linux/UE4Editor; file=`pwd`/$1; $bin $file; }
UE4GenerateProjectFiles() {
  bin=${UE4}/GenerateProjectFiles.sh; file=`pwd`/$1;
  $bin -project=${file} -game -engine;
}
```

- Generate project file and use Makefile

```bash
UE4GenerateProjectFiles playground.uproject
make playgroundEditor
# or make playgroundEditor-Linux-Debug
UE4Editor playground.uproject
```

### Mac

TODO: Need help
<!-- TODO -->

## Next

After finishing setting up the development environment, you can write code to do more tasks beyond UnrealCV already provides. Tutorial [add a new command](plugin/add-command.md) shows how to add a new UnrealCV command to the plugin.

Useful resources for development are:
- [Code documentation](plugin/api-docs.md)
- [Project architecture](reference/arch.md)

This page briefly describes how to use UnrealCV as an Unreal Engine plugin. Make sure you read [Getting Started](/basic/getting-started.md) before trying to use UnrealCV plugin.

Install
=======
If you are a windows or mac user, we recommend method 1. If you are a Linux user or want to try a different version from the marketplace, we recommend 2. If you want to try a cutting edge version, you can always compile the plugin from source code.

1. Install from UE4 marketplace (coming)

    For Windows and Mac user, we are working on pushing UnrealCV to the UE4 marketplace, so that the installation can be as simple as clicking a button.

    ---

2. Download compiled binaries

    Install UnrealCV is copying compiled UnrealCV binaries to the plugins folder of an UE4 project or to the UE4 engine path. UnrealCV binaries can be downloaded from the [release page](https://github.com/unrealcv/unrealcv/releases).

    The compiled version is provided for the latest version of UE4 that can well support Linux.

## Compile from source code (advanced)

If you want to try a version of UnrealCV not provided in our [release page](), for example if you want to try some experimental features in a branch, compiling the plugin code from source is an way.

To compile UnrealCV plugin you can use

```
python client/scripts/build-plugin.py --engine_path [UnrealEnginePath] --dev
```

The default UnrealEnginePath:

- Mac: `/Users/Shared/UnrealEngine/[version]`
- Windows: TODO
- Linux: TODO

The plugin binaries will be produced in `client/scripts/tmp` folder. Then you can copy the compiled plugin to a UE4 project.

After that you should see `Automation.Execute: BUILD SUCCESSFUL`


If you want to modify UnrealCV code and add new features to it. Please refer to the page describing how to setup a dev environment [Plugin - Development](/plugin/dev.md#setup). Setting up a dev environment takes more time but makes it much easier to debug and modify code.

'''tip
    When using in the UE4 editor, it is strongly recommend to turn off `Editor Preference -> General -> Misc. -> Use Less CPU when in Background`

## Configuration
There are a few configurations of unrealcv can be changed.

These configuration can be changed by modifying a configuration file, the configuration file is exactly the same for the `binary` and `plugin`.

---
## Tested projects

Unreal Engine projects are of dramatic different scales and complexity. UnrealCV is far from perfect and it has compatible issues with some Unreal Projects due to various reasons. Here are a few projects we have already verifed that UnrealCV can work well with. If you want us to test a map (project), please let us know.

Here are a list of Unreal projects that we tried and verified.

- [Playground](), tested by @qiuwch.
- [Realistic Rendering](), tested by @qiuwch.
- [CityScene](), tested by @qiuwch, @edz-o
- [SunTemple](), tested by @edz-o

---
## Issues and workarounds

UnrealCV is far from perfect, currently it has some limitations and these are workaround for solving them. We will likely to fix these issues and make this section not unnecessary.

Issues and workaround can be found in [issue tracker]().

```Under construction```

This page briefly describes how to install UnrealCV to Unreal Engine. Make sure you read [Getting Started](/binary/getting-started.md) before using UnrealCV plugin.

UnrealCV plugin can add extra functions to Unreal Engine Editor.

## Download compiled binaries

Install UnrealCV is just copying compiled UnrealCV binaries to the plugins folder of an UE4 project or to the UE4 engine path. UnrealCV binaries can be downloaded from the [release page](https://github.com/unrealcv/unrealcv/releases).

## Install from UE4 marketplace (coming)

For Windows and Mac user, we are working on pushing UnrealCV to the UE4 marketplace, so that the installation can be as simple as clicking a button.

## Compile from source code

If you want to try a version of UnrealCV not provided in the [release page](), for example if you want to try some experimental features in a branch, compiling the plugin code from source is an way.

To compile UnrealCV plugin you can use
```
python client/script/build-plugin.py --engine_path [UnrealEnginePath] --dev
```
The plugin binaries will be produced in `tmp` folder.


If you want to modify UnrealCV code and add new features to it. Please refer to the page describing how to setup a dev environment [here](/dev/setup.md). Setting up a dev environment takes more time but makes it much easier to debug and modify code.

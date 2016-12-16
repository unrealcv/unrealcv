# Plugin installation and usage

This page briefly describes how to install UnrealCV plugin as an Unreal Engine plugin. Make sure you read [getting started](basics.md) before trying to use the plugin.

## Install

1. Install from UE4 marketplace \(coming\)

   For Windows and Mac user, UnrealCV will be able to installed from the UE4 marketplace. We are still finalizing the submission to the UE4 marketplace and it will be available soon.

2. Use compiled plugin binary

    You can download compiled UnrealCV binaries from our [release page](https://github.com/unrealcv/unrealcv/releases) and copy the compiled binaries to the plugins folder to install it. The compiled binaries is provided for the latest version of UE4 that [can support Linux](https://wiki.unrealengine.com/Building_On_Linux), currently it is 4.14. If you need a version other than that, you can [send us a request](contact.md) or build it yourself.

    - Install to project
        Go to project folder which contains `[ProjectName].uproject`
        Create a folder called `Plugins`
        Put `UnrealCV` folder into the `Plugins` folder.
    - Install to Unreal Engine
        Go to the plugin folder of Unreal Engine which is `Engine/Plugins`
        Put `UnrealCV` folder into the `Plugins` folder.

Open `Menu -> Edit -> Plugins`, make sure UnrealCV is installed and enabled.

![plugin_info](assets/plugin.png)

## Special tips for Linux

In Linux, the Unreal Engine needs to be built from source code. How to compile from source code can be found in this official document [Building On Linux](https://wiki.unrealengine.com/Building_On_Linux). But the Linux version currently does not contain OpenEXR support, which is required for getting accurate depth.

To solve this, download our [OpenEXR patch for linux](https://unrealcv.github.io/files/0001-Fix-openexr-support-for-linux-version.patch) and run `git apply 0001-Fix-openexr-support-for-linux-version.patch` after running `./GenerateProjectFiles.sh`.

## Compile from source code \(advanced\)

**Build the plugin**
UnrealEngine provides UAT script to compile a plugin. This script will embed the plugin into a dummy project then compile it. If you want to try a version of UnrealCV not provided in our [release page](https://github.com/unrealcv/unrealcv/releases). For example, you want to try some experimental features in a branch, compiling the plugin code from source code is the only choice.

To compile UnrealCV plugin, use `build.sh` for Linux and Mac, use `build.bat` for windows. After running this command you should see `Automation.Execute: BUILD SUCCESSFUL`

The [UnrealEnginePath] is the place that contains the `Engine` folder of UE4. For example, in my machine, it is  `/Users/Shared/UnrealEngine/4.12` in Mac, `D:\Epic Games\4.13` in windows. The plugin binaries will be produced in the `built`. Then you can copy the compiled plugin to a UE4 project.

If you want to modify UnrealCV code and add new features. Please refer to the [development page](/plugin/dev.md). Setting up a dev environment takes more time but makes it much easier to debug and modify code.

### Tips

    When using the plugin the UE4 editor, it is strongly recommend to turn off the setting `Editor Preference -> General -> Misc. -> Use Less CPU when in Background`.

## Configuration

There are a few configurations of UnrealCV can be changed. These configuration can be changed by modifying a configuration file, the configuration file is exactly the same as described in the [game mode](/unrealcv-basics.md)

## Tested projects

Unreal Engine projects are of dramatic different scales and complexity. It can be as simple as just a single room, or be a large city or outdoor scene. UnrealCV is far from perfect and it has compatible issues with some Unreal projects. Here are a few projects we are currently using and have verified that UnrealCV can work well with. If you want us to test a map \(project\), please let us know.

Here are a list of Unreal projects that we tried and verified.

* [Playground](), tested by @qiuwch.
* [Realistic Rendering](), tested by @qiuwch.
* [CityScene](), tested by @qiuwch, @edz-o
* [SunTemple](), tested by @edz-o

## Issues and workarounds

Issues and workaround can be found in [issue tracker](https://github.com/unrealcv/unrealcv/issues). Please use the search function before posting your issue, your problem might have already been answered. Also you are welcome to chat with us in our [gitter channel](https://gitter.im/unrealcv/unrealcv).

If the plugin crashed the editor, please send us your crash log to help us figure out what is going wrong. The crash log can be found in `Saved/CrashReport`. If you can not find the crash report, you can also send us the core dump file.

- The speed of UnrealCV

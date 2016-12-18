<!-- Make this page shorter -->

This page briefly describes how to install UnrealCV plugin as an Unreal Engine plugin. Make sure you read [getting started](/getting-started.md) before trying to use the plugin.

<h3>Use compiled plugin binary</h3>

You can download compiled UnrealCV binaries from our [release page](https://github.com/unrealcv/unrealcv/releases). Then copy the compiled binaries to the plugins folder to install it. The compiled binaries is provided for UE4.13. If you need a version other than that, you can [send us a request](/contact.md) or build it yourself by following the instruction. You can install the plugin to either a game project or to UE4 engine.

- Install to project     
    - Go to project folder which contains `[ProjectName].uproject`
    - Create a folder called `Plugins`
    - Put `UnrealCV` folder into the `Plugins` folder.
- Install to Unreal Engine
    - Go to the plugin folder of Unreal Engine which is `Engine/Plugins`
    - Put `UnrealCV` folder into the `Plugins` folder.

Open `Menu -> Edit -> Plugins`, make sure UnrealCV is installed and enabled.

![plugin_info](/images/plugin.png)

<h3>Install from UE4 marketplace (coming)</h3>

   For Windows and Mac user, UnrealCV will be available from the UE4 marketplace. We are still finalizing the submission to the UE4 marketplace and it will be available soon.

## Special tips for Linux

In Linux, the Unreal Engine needs to be built from source code. How to compile from source code can be found in this official document [Building On Linux](https://wiki.unrealengine.com/Building_On_Linux). But the Linux version currently does not contain OpenEXR support, which is required for getting accurate depth.

To solve this, download our [OpenEXR patch for linux](https://unrealcv.github.io/files/0001-Fix-openexr-support-for-linux-version.patch) and run `git apply 0001-Fix-openexr-support-for-linux-version.patch` after running `./GenerateProjectFiles.sh`. This dependency will be removed later.

## Compile from source code \(advanced\)

If you want to try a version of UnrealCV not provided in our [release page](https://github.com/unrealcv/unrealcv/releases). For example, you want to try some experimental features, compiling the plugin code from source code is the only choice.

To compile UnrealCV plugin, use `build.sh` for Linux and Mac, use `build.bat` for windows, remember to set the path of Unreal Engine by following instructions of the script. After running this command you should see `Automation.Execute: BUILD SUCCESSFUL`. The plugin binaries will be produced in the `built`. Then you can copy the compiled plugin to a UE4 project.

If you want to modify UnrealCV code and add new features. Please refer to the [development setup](/plugin/dev.md). Setting up a dev environment takes a bit of time but will make it much easier to debug and modify code.

!!! Tip
    When using the plugin in the editor, it is strongly recommend to turn off the setting `Editor Preference -> General -> Misc. -> Use Less CPU when in Background`.

## Configuration

There are a few configurations of UnrealCV can be changed. These configurations can be changed by modifying a configuration file, the configuration file is exactly the same as described in the [game mode](/game/configuration.md)

## Verified projects

Unreal Engine projects are of dramatic different scales and complexity. It can be as simple as just a single room, or be a large city or outdoor scene. UnrealCV is far from perfect and it has compatible issues with some Unreal projects. Here are a few projects we are currently using and have verified that UnrealCV can work well with. If you want us to test a map \(project\), please let us know.

Here are a list of Unreal projects that we tried and verified.

* [Playground](), tested by @qiuwch.
* [Realistic Rendering](), tested by @qiuwch.
* [CityScene](), tested by @qiuwch, @edz-o
* [SunTemple](), tested by @edz-o

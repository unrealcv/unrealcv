================
Trouble Shooting
================

We tried our best to make the software stable and easy to use, but accidents sometimes happen. Here are a list of issues that you might find. Use the search function :kbd:`ctrl+f` to search your error message. If you can not find useful information here, post a new issue.

- The binary can not run

python3 support. See issue :issue:`49`, Thanks to :user:`jskinn` for the idea!

.. _supported:

Supported Unreal Engine Version
===============================================

UE4 (4.12, 4.13, 4.14)

Client
------
Python
MATLAB

4.14
----
Windows
Linux
Mac

.. TODO: Add missing details


Verified projects
-----------------

Unreal Engine projects are of dramatic different scales and complexity. It can be as simple as just a single room, or be a large city or outdoor scene. UnrealCV is far from perfect and it has compatible issues with some Unreal projects. Here are a few projects we are currently using and have verified that UnrealCV can work well with. If you want us to test a map (project), please let us know.

Here are a list of Unreal projects that we tried and verified.

- Playground, tested by @qiuwch.
- Realistic Rendering, tested by @qiuwch.
- CityScene, tested by @qiuwch, @edz-o
- SunTemple, tested by @edz-o

Known issues
============
- The binary can not run

    For example an error like this.

.. code::

    [2017.05.25-04.14.33:476][  0]LogLinux:Error: appError called: Assertion failed: Assertion failed:  [File:/UE4/Engine/Source/Runtime/OpenGLDrv/Private/Linux/OpenGLLinux.cpp] [Line: 842]
Unable to dynamically load libGL: Could not retrieve EGL extension function eglQueryDevicesEXT

    It is very likely an issue with the OpenGL of the system.

    :code:`sudo apt-get install mesa-utils` and run :code:`glxgears`. Make sure you can see a window with gears running in it.

- The screen resolution is not what I want

    - In editor, change `Editor preference -> Level Editor -> Play`
    - In the game mode, use console command `setres 640x480`
    - Change the config file shown in `vget /unrealcv/status`

- The speed of UnrealCV

- The OpenEXR requirement

- The Unreal Engine config file not changed

- The image and ground truth not aligned

Build
=====

Mac
---
Native error= Cannot find the specified file

https://answers.unrealengine.com/questions/574562/cannot-package-a-plugin-in-415mac.html

Invalid SDK MacOSX.sdk, not found in /Library/Developer/CommandLineTools/Platforms/MacOSX.platform/Developer/SDKs

https://answers.unrealengine.com/questions/316117/missing-project-modules-1.html
https://github.com/nodejs/node-gyp/issues/569#issuecomment-255589932

Issues and workarounds
======================

Issues and workaround can be found in [issue tracker](https://github.com/unrealcv/unrealcv/issues). Please use the search function before posting your issue, your problem might have already been answered. Also you are welcome to chat with us in our [gitter channel](https://gitter.im/unrealcv/unrealcv).

If the plugin crashed the editor, please send us your crash log to help us figure out what is going wrong. The crash log can be found in `Saved/CrashReport`. If you can not find the crash report, you can also send us the core dump file.


- Can not connect to the binary

Use :code:`vget /unrealcv/status` to make sure the server is listening and no client is connected.

Subscribe to an issue if you want to get future notification.

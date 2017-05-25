======
Issues
======

We tried our best to make the software stable and easy to use, but accidents sometimes happen. Here are a list of issues that you might find. Use the search function :kbd:`ctrl+f` to search your error message. If you can not find useful information here, post a new issue.

- The binary can not run

:issue: #30

Error message..

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

- The speed of UnrealCV

- The OpenEXR requirement

- The Unreal Engine config file not changed

- The image and ground truth not aligned

Issues and workarounds
======================

Issues and workaround can be found in [issue tracker](https://github.com/unrealcv/unrealcv/issues). Please use the search function before posting your issue, your problem might have already been answered. Also you are welcome to chat with us in our [gitter channel](https://gitter.im/unrealcv/unrealcv).

If the plugin crashed the editor, please send us your crash log to help us figure out what is going wrong. The crash log can be found in `Saved/CrashReport`. If you can not find the crash report, you can also send us the core dump file.


- Can not connect to the binary

Use :code:`vget /unrealcv/status` to make sure the server is listening and no client is connected.

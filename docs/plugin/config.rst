The configuration file
======================

Start from UnrealCV ``v0.3.1``, the config of UnrealCV can be configured in a configuration file. Use :code:`vget /unrealcv/status` to see current configuration.

Change game resolution
----------------------

The output resolution of UnrealCV is independent of the window size.

If you want to change the display resolution.
In game mode, use console command :code:`r.setres 320x240`

When use ``play -> selected viewport`` the resolution can not be changed, use `play -> new window editor` instead.

Change the server port
----------------------
Use :code:`vget /unrealcv/status` to get the directory of the configuration file. Then open the configuration file and modify the server port.

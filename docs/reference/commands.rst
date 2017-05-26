Command System
==============

Unreal Engine 4 has some built-in commands to help game development. These commands can be typed into a built-in console. Using these commands, a developer can profile the game performance and view debug information.  To invoke the built-in console of a game, type the \` key (the key above tab).

UnrealCV provides commands useful for computer vision researchers. What is more, these commands can be used by an external program. A built-in command can also be used using the special command :code:`vrun`.

Command cheatsheet
------------------

.. ::note::

    This command list is growing and subject to change

Reply [this thread](https://groups.google.com/d/topic/unrealcv/EuJlibmTN3c/discussion) to tell us what missing functions are needed for your project. We will consider adding it in the future release.


1. Camera operation
-------------------

See :code:`Source/UnrealCV/Private/Commands/CameraHandler.h(.cpp)` for more details.

vget /camera/[id]/location
    (v0.2) Get camera location [x, y, z]

vget /camera/[id]/rotation
    (v0.2) Get camera rotation [pitch, yaw, roll]

vset /camera/[id]/location [x] [y] [z]
    (v0.2) Set camera location [x, y, z]

vset /camera/[id]/rotation [pitch] [yaw] [roll]
    (v0.2) Set camera rotation [pitch, yaw, roll]

vget /camera/[id]/[viewmode]
    (v0.2) Get [viewmode] from the [id] camera, for example: vget /camera/0/depth

vget /camera/[id]/[viewmode] [filename]
    (v0.2) Same as the above, with an extra parameter for filename

    :filename: Filename is where the file will be stored.
    :example: :code:`vget /camera/0/lit lit.png`

vget /camera/[id]/[viewmode] [format]
    (v0.3.7) Support binary data format

    :format: If only file format is specified, the binary data will be returned through socket instead of being saved as a file.
    :example: :code:`vget /camera/0/lit png`

vget /camera/[id]/object_mask
    (v0.2) The object mask is captured by first switching the viewmode to object_mask mode, then take a screenshot

vset /viewmode [viewmode]
    (v0.2) Set ViewMode to (lit, normal, depth, object_mask)

vget /viewmode
    (v0.2) Get current ViewMode

2. Object interaction
---------------------

See `Source/UnrealCV/Private/Commands/ObjectHandler.h(.cpp)` for more details

vget /objects
    (v0.2) Get the name of all objects

vget /object/[obj_name]/color
    (v0.2) Get the labeling color of an object (used in object instance mask)

vset /object/[obj_name]/color [r] [g] [b]
    (v0.2) Set the labeling color of an object

3. Plugin commands
------------------

See `Source/UnrealCV/Private/Commands/PluginHandler.h(.cpp)` for more details.

vget /unrealcv/status
    (v0.2) Get the status of UnrealCV plugin

vget /unrealcv/help
    (v0.2) List all available commands and their help message

4. Action commands
------------------

See :file:`Source/UnrealCV/Private/Commands/ActionHandler.h(.cpp)`

vset /action/keyboard [key_name] [delta]
    (v0.3.6) Valid key_name can be found in `here <https://wiki.unrealengine.com/List_of_Key/Gamepad_Input_Names>`__

Run UE4 built-in commands
-------------------------

vrun [cmd]
    (v0.3) This is a special command used to execute Unreal Engine built-in commands. UE4 provides some built-in commands for development and debug. They are not very well documented, but very useful.

A few examples are:

- :code:`stat FPS` - show current frame rate
- :code:`shot` - take a screenshot
- :code:`show Material` - toggle the display of Material

These commands can be executed in the UE4 console. If you want to use them in UnrealCV, you can prefix these commands with `vrun stat FPS`.

Run Blueprint commands
----------------------

vexec [cmd]
    TODO

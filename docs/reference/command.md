# UnrealCV commands
The function of UnrealCV is provided by a set of UnrealCV commands.

## The command system
<!-- Better help system -->

Unreal Engine 4 has some built-in commands to help game development. These commands can be typed into a built-in console. Using these commands, a developer can profile the game performance and view debug information.  To invoke the built-in console of a game, type the <code>&#96;</code> key (the key above tab).

UnrealCV adds a set of commands useful for computer vision research. What is more, these commands can be used by an external program.

Too powerful, abstract


## Command cheatsheet

<blockquote>
This command list is growing and subject to change
</blockquote>

[Contact us](contact.md) to tell us what missing functions are needed for your project. We will consider adding it in the future release.

<!-- Do not use table, very hard to extend, this is manually synced from the source code -->

## Camera operation

See `Source/UnrealCV/Private/Commands/CameraHandler.h` and `Source/UnrealCV/Private/Commands/CameraHandler.cpp` for more details.

- `vget /camera/[id]/location`

    (v0.2) Get camera location [x, y, z]

- `vget /camera/[id]/rotation`

    (v0.2) Get camera rotation [pitch, yaw, roll]

- `vset /camera/[id]/location [x] [y] [z]`

    (v0.2) Set camera location [x, y, z]

- `vset /camera/[id]/rotation [pitch] [yaw] [roll]`

    (v0.2) Set camera rotation [pitch, yaw, roll]

- `vget /camera/[id]/[viewmode]`

    (v0.2) Get [viewmode] from the [id] camera, for example: vget /camera/0/depth

- `vget /camera/[id]/[viewmode] [filename]`

    (v0.2) Same as the above, with an extra parameter for filename

- `vset /viewmode [viewmode]`  

    (v0.2) Set ViewMode to (lit, normal, depth, object_mask)

- `vget /viewmode`

    (v0.2) Get current ViewMode

## UnrealCV plugin commands

See `Source/UnrealCV/Private/Commands/PluginHandler.h`
### Defined in `Private/Commands/PluginHandler.cpp`

- `vget /unrealcv/status`

    (v0.2) Get the status of UnrealCV plugin

- `vget /unrealcv/help`

    (v0.2) List all available commands and their help message


## Object interaction

See `Source/UnrealCV/Private/Commands/ObjectHandler.h` and `Source/UnrealCV/Private/Commands/ObjectHandler.cpp` for more details

- `vget /objects`

    (v0.2) Get the name of all objects

- `vget /object/[obj_name]/color`

    (v0.2) Get the labeling color of an object (used in object instance mask)

- `vset /object/[obj_name]/color [r] [g] [b]`

    (v0.2) Set the labeling color of an object

## Run UE4 built-in commands

- `vrun [cmd]`

    (v0.3) This is a special command used to execute Unreal Engine built-in commands. UE4 provides some built-in commands for development and debug. They are not very well documented, but very useful.

    A few examples are:
    `stat FPS` - show current frame rate
    `shot` - take a screenshot
    `show Material` - toggle the display of Material

    These commands can be executed in the UE4 console. If you want to use them in UnrealCV, you can prefix these commands with `vrun stat FPS`.

Command System
==============

The machine-readable command schema is generated from server registrations and committed in ``docs/reference/command_schema.json``.
Use ``python tools/command_schema/generate_schema.py`` to regenerate it after command changes.

This page is the command contract for the open-source UnrealCV plugin in this
repository. The generated index contains only commands registered by this
checkout. Features tested first in UnrealZoo are listed separately in
:doc:`../unrealcv_plus/reference/commands` under the
**UnrealCV Dev For UnrealZoo** availability label.

Runtime capability detection
----------------------------

Do not infer server support from a client method or documentation version.
Query the connected server and compare exact templates::

    from unrealcv import Client

    client = Client(('127.0.0.1', 9000))
    client.connect()
    templates = set(client.request('vget /unrealcv/commands').splitlines())

    required = 'vget /camera/[uint]/lit_shared'
    if required not in templates:
        raise RuntimeError(f'The connected UnrealCV build does not provide {required}')

The committed :file:`command_schema.json` and the generated index at the end of
this page are the authoritative build-time inventory for the current branch.

Unreal Engine has built-in commands to help game development. These commands can be typed into the built-in console. Using these commands, a developer can profile game performance and view debug information. To invoke the console, type the \` key (the key above tab).

UnrealCV provides commands useful for computer vision researchers. What is more, these commands can be used by an external program. A built-in command can also be used using the special command :code:`vrun`.

Console command completion
--------------------------

When the Unreal Engine console is open, type an UnrealCV verb such as :code:`vget` or :code:`vset` to see registered command suggestions. Each suggestion includes the command template and the first line of its help text. UnrealCV also provides executable argument examples and camera-index examples such as :code:`vget /camera/0/location`.

The completion list is generated from the live command dispatcher and refreshed after UnrealCV registers its command handlers, so newly registered commands appear without maintaining a separate console command list.

Command cheatsheet
------------------

.. ::note::

    This command list is growing and subject to change

See this `ipython notebook <https://github.com/unrealcv/unrealcv/blob/master/examples/commands_demo.ipynb>`__ to see an imcomplete demo of available commands.

.. Reply [this thread](https://groups.google.com/d/topic/unrealcv/EuJlibmTN3c/discussion) to tell us what missing functions are needed for your project. We will consider adding it in the future release.


1. Camera operation
-------------------

See :file:`Source/UnrealCV/Private/Commands/CameraHandler.h(.cpp)` for more details.

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

vget /camera/[id]/lit_shared
    (v1.1.0) Capture the lit image into a Windows named shared-memory region and return JSON metadata containing its name, shape, dtype, and byte size.

vget /camera/[id]/depth_shared
    (v1.1.0) Capture the depth image into Windows shared memory as a row-major ``float32`` array and return JSON metadata.

vget /camera/[id]/normal_shared
    (v1.1.0) Capture the surface-normal image into Windows shared memory as BGRA ``uint8`` pixels and return JSON metadata.

vget /camera/[id]/object_mask_shared
    (v1.1.0) Capture the object mask into Windows shared memory as BGRA ``uint8`` pixels and return JSON metadata.

vget /camera/[id]/seg_shared
    (v1.1.0) Alias of ``object_mask_shared`` for capturing the segmentation mask into Windows shared memory.

vset /viewmode [viewmode]
    (v0.2) Set ViewMode to (lit, normal, depth, object_mask)

vget /viewmode
    (v0.2) Get current ViewMode

vget /camera/[id]/pose
    (v0.3.10) Get camera location [x, y, z] and rotation [pitch, yaw, roll]

vset /camera/[id]/pose [x] [y] [z] [pitch] [yaw] [roll]
    (v0.3.10) Teleport camera to location [x, y, z] and rotation [pitch, yaw, roll]

vget /camera/[id]/horizontal_fieldofview
    (v0.3.10) Get camera horizontal field of view. This cmd is deprecated, use vget /camera/[uint]/fov instead (v0.4.0).

vset /camera/[id]/horizontal_fieldofview [FOV]
    (v0.3.10) Set camera horizontal field of view. This cmd is deprecated, use vset /camera/[uint]/fov [FOV] instead (v0.4.0).

vget /camera/[id]/vis_depth npy
    (v0.3.10)

vget /camera/[id]/plane_depth npy
    (v0.3.10)

vget /cameras
    (v0.4.0) List all cameras in the scene

vset /cameras/spawn
    (v0.4.0) Spawn a new camera

vset /camera/[id]/size [width] [height]
    (v0.4.0) Set the size of the camera image

vget /camera/[id]/size
    (v0.4.0) Get the size of the camera image

vset /camera/[id]/projection_type [type]
    (v0.4.0) Set the projection type of the camera, type can be perspective or orthographic

vset /camera/[id]/ortho_width [width]
    (v0.4.0) Set the orthographic width of the camera

2. Object interaction
---------------------

See :file:`Source/UnrealCV/Private/Commands/ObjectHandler.h(.cpp)` for more details

vget /objects
    (v0.2) Get the name of all objects

vget /object/[obj_name]/color
    (v0.2) Get the labeling color of an object (used in object instance mask)

vset /object/[obj_name]/color [r] [g] [b]
    (v0.2) Set the labeling color of an object

vset /object/[obj_name]/show
    (v0.3.10) Show object

vset /object/[obj_name]/hide
    (v0.3.10) Hide object

vget /object/[obj_name]/mobility
    (v0.3.10) Get object mobility

vget /object/[obj_name]/material
    (editor) List material path for each slot on the first StaticMeshComponent on the object

vset /object/[obj_name]/material [slot] [material_path]
    (editor) Assign ``material_path`` to ``slot`` on the first StaticMeshComponent (UE object path, e.g. ``/Game/Materials/M_Material.M_Material``)

vset /objects/spawn [class_name] [obj_name]
    (v0.4.0) Spawn an object with class name and object name. It can also be used to create a new camera, for example:

    - :code:`vset /objects/spawn FusionCameraActor Cam_1` - create a new camera named Cam_1
    - :code:`vset /objects/spawn StereoCameraActor StereoCam_1` - create a new stereo camera named StereoCam_1

vset /objects/spawn [class_name] [x] [y] [z]
    (v1.1.0) Spawn an object at a world-space location.

vset /objects/spawn [class_name] [obj_name] [x] [y] [z]
    (v1.1.0) Spawn a named object at a world-space location. The same coordinate forms are available for :code:`vset /objects/spawn_cube`.

vset /object/[obj_name]/destroy
    (v0.4.0) Destroy object

vset /object/[obj_name]/name [new_obj_name]
    (v0.4.0) Rename object

vget /object/[obj_name]/scale
    (v0.4.0) Get object scale

vset /object/[obj_name]/scale [x] [y] [z]
    (v0.4.0) Set object scale

vset /object/[obj_name]/name [new_obj_name]
    (v0.4.0) Rename object

vget /object/[obj_name]/uclass_name
    (v0.4.0) Get UClass name of an object

vget /object/[obj_name]/bounds
    (v0.4.0) Get object bounds in the world coordinate, format is [minx, y, z, maxx, y, z]

vget /object/[obj_name]/vertex_location
    (v0.4.0) Get the vertex location of an object

vget /object/[obj_name]/bones
    (v1.1.0) Return all bone transforms in component space.

vget /object/[obj_name]/bones [component|world]
    (v1.1.0) Return all bone transforms in the selected coordinate space.

vget /object/[obj_name]/bones [bone_1,bone_2,...] [component|world]
    (v1.1.0) Return skeletal or poseable mesh bone transforms as JSON. The default is all bones in component space. This command provides world-space bone locations for pose and keypoint workflows. If an actor has multiple eligible mesh components, only the first poseable mesh component, or otherwise the first skeletal mesh component, is queried.

3. Plugin commands
------------------

See :file:`Source/UnrealCV/Private/Commands/PluginHandler.h(.cpp)` for more details.

vget /unrealcv/status
    (v0.2) Get the status of UnrealCV plugin

vget /unrealcv/help
    (v0.2) List all available commands and their help message

vget /unrealcv/commands
    Return all command templates currently registered in the UnrealCV command
    dispatcher. The result is stable, sorted plain text with exactly one command
    template per line and no heading or description. Clients can use this output
    for runtime capability detection.

vget /unrealcv/version
    (v0.3.10) Get the version of UnrealCV plugin

vget /scene/name
    (v0.3.10) Get the name of this scene

vget /level/name
    (v0.3.10) Get the name of the current level

4. Action commands
------------------

See :file:`Source/UnrealCV/Private/Commands/ActionHandler.h(.cpp)`

vset /action/keyboard [key_name] [delta]
    (v0.3.6) Valid key_name can be found in `here <https://wiki.unrealengine.com/List_of_Key/Gamepad_Input_Names>`__

vset /action/game/pause
    (v0.3.10) Pause the game

vget /action/game/is_paused
    (v0.3.10) Check if the game is paused

vset /action/game/resume
    (v0.3.10) Resume the game

vset /action/game/level [level_name]
    (v0.3.10) Open a new level

vset /action/input/enable
    (v0.3.10) Enable input

vset /action/input/disable
    (v0.3.10) Disable input

vset /action/eyes_distance [eye_distance]
    (v0.3.10) Set the eye distance between left eye and right eye (camera 1). This command might be marked as deprecated when we finish multiple camera support.

5. Run Unreal Engine built-in commands
--------------------------------------

vrun [cmd]
    (v0.3) This is a special command used to execute Unreal Engine built-in commands. Unreal Engine provides built-in commands for development and debugging.

A few examples are:

- :code:`stat FPS` - show current frame rate
- :code:`shot` - take a screenshot
- :code:`show Material` - toggle the display of Material

These commands can be executed in the Unreal Engine console. To use one through UnrealCV, prefix it with ``vrun``, for example ``vrun stat FPS``.

6. Run Blueprint commands
--------------------------

vbp [obj_name] [func_name] [arg1] [arg2] ...
    (v0.4.0) This is a special command used to execute Blueprint commands. Blueprint is Unreal Engine's visual programming language. UnrealCV provides a way to call Blueprint functions from the command line.
A few examples are:
 - :code:`vbp BP_Player_C GetActorLocation` - Get the location of the player
 - :code:`vbp BP_Player_C SetActorLocation 100 200 300` - Set the location of the player

Note that the Blueprint function name is case sensitive, depending on how it is defined in the Blueprint editor.

7. Runtime reflection
---------------------

``vreflect`` exposes selected Unreal reflection operations through the UnrealCV command channel.

vreflect [obj_name] functions
    List reflected functions on an object as JSON.

vreflect [obj_name] properties
    List reflected properties on an object as JSON.

vreflect [obj_name] get [property_path]
    Read a property by name or dotted path and return its type and value as JSON.

vreflect [obj_name] set [property_path] [value]
    Set a property using Unreal text import syntax and return the updated value.

vreflect [obj_name] call_json [function_name] [json_args]
    Call a reflected function with a JSON object keyed by parameter name and return output parameters as JSON.

Prefix the target with ``class:`` or ``cdo:`` to address a class default object. Examples:

- :code:`vreflect BP_Player_C_0 get RootComponent.RelativeLocation`
- :code:`vreflect BP_Player_C_0 set RootComponent.RelativeLocation "(X=100,Y=200,Z=300)"`
- :code:`vreflect class:KismetMathLibrary call_json Add_IntInt {"A":2,"B":3}`

Only expose UnrealCV to trusted clients. Reflection can read and mutate runtime state and invoke reflected functions.

8. Complete registered command index
------------------------------------

The following generated index is synchronized with every production ``BindCommand`` registration. It supplements the hand-written explanations above and is the authoritative list for the current branch.

.. include:: commands_generated.rst.txt

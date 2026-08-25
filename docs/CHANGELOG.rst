=========
CHANGELOG
=========

Development branch
==================

- v1.1.0 (2026-08-12)
    - Modernize the UnrealCV plugin and server for the Unreal Engine 5 development line.
    - Add optical-flow capture and expand camera projection and capture controls, including image size, capture source, exposure, motion blur, focal distance, and depth of field.
    - Add local Windows shared-memory transport for lit, depth, normal, and object-mask images, with structured metadata describing each frame.
    - Add camera spawning, explicit-position object and cube spawning, skeletal and poseable-mesh bone queries, material inspection and assignment, object labels, class queries, destruction, bounds, scale, and other object controls.
    - Add runtime command discovery through :code:`vget /unrealcv/commands`, live console command completion, and reflection-based property and function inspection through :code:`vreflect`.
    - Add generated command-schema and documentation-coverage checks, together with workflow tooling for building, launching, monitoring, testing, and benchmarking UnrealCV environments.
    - Expand the Python client with a typed high-level API, structured models, capability checks, improved reconnect and error handling, modern packaging, and broader transport and API regression tests.
    - Harden TCP disconnect, reset, framing, dispatch, and null-state handling; add bind-address, authentication, and command-policy controls.
    - This tag marks the UnrealCV plugin/server 1.1.0 baseline. The Python client state is included as of this commit, but its package metadata is not asserted to match the server version.

- v1.0.1 (2023-07-13)
    - Collect the UnrealCV development work accumulated since v0.3.10 into a new plugin release line.
    - Introduce the component-based camera and sensor architecture, including multiple-camera, stereo, and fusion-camera workflows.
    - Expand sensing modes with lit, depth, normal, annotation, vertex, bone, and transparent-object depth capture.
    - Add :code:`DataCaptureActor` and :code:`Puppeteer` workflows for synchronized scene, camera, animation, pose, vertex, and image-data collection.
    - Expand camera controls for field of view, image size, movement, pose, projection data, and additional capture formats.
    - Expand object controls for spawning, renaming, scale, bounds, visibility, transforms, and annotation color.
    - Add same-frame batched command execution and Blueprint function invocation through :code:`vbp`.
    - Add Linux communication support, configurable server ports through :code:`-cvport`, and runtime texture loading from files.
    - Reorganize the C++ plugin into sensor, controller, server, actor, and utility modules, and refresh the Python client and documentation.
    - Projects that include UnrealCV internal C++ headers or depend on legacy capture and sensor classes should review the renamed and removed interfaces before upgrading.

- Python client 0.4.0 (2019-07-15; no UnrealCV plugin/server tag)
    - Update the Python package version from 0.3.10 to 0.4.0.
    - Add batch command execution support during this development period and fix a compilation issue caused by branch restructuring.
    - The UnrealCV plugin/server version remained 0.3.10; this is documented separately rather than represented by a :code:`v0.4.0` plugin tag.
- v0.3.10
    - Commands contributed in pull request :issue:`91`, authored by :user:`bennihepp`
        - Add :code:`vget /camera/[id]/pose`, :code:`vset /camera/[id]/pose`
        - Add :code:`vget/vset /camera/[id]/horizontal_fieldofview`
        - Add :code:`vget /camera/[id]/vis_depth npy` and :code:`vget /camera/[id]/plane_depth npy`
        - Add :code:`vset /object/[id]/show`, :code:`vset /object/[id]/hide`
        - Add :code:`vset /action/input/enable`, :code:`vset /action/input/disable`

    - Add more commands
        - Add :code:`vget /object/[id]/mobility`, :code:`vget /object/[id]/location`, :code:`vget /object/[id]/rotation`
        - Add :code:`vget /camera/[id]/normal npy`
        - Add :code:`vset /action/eyes_distance [eye_distance]`
        - Add :code:`vset /action/game/pause`

    - Update the python client to support python3
    - Improve documentation

- v0.3.9
    - Fix a bug that prevents object mask generation, which is introduced in v0.3.7
    - Fix #53 that the painting of object does not work
    - Fix #49 python3 support, thanks to @jskinn and @befelix
- v0.3.8 :
    - Integrate cnpy into unrealcv
    - Add :code:`vget /camera/depth npy`, which can return tensor as a numpy binary.
- v0.3.7 :
    - Add :code:`vget /camera/lit png` to retrieve binary data without saving it.
- v0.3.6 :
    - Change docs from markdown to reStructuredText
    - Add docker to automate tests
    - Add :code:`vset /action/keyboard [key_name] [delta]`
- v0.3.5 : Add vexec to support the invocation of blueprint functions, Add :code:`GetWorld()` in :code:`FCommandHandler`.
- v0.3.4 : Delay the object mask painting from initialization code
- v0.3.3 : Add :code:`vget /scene/name`
- v0.3.2 :
    - Add :code:`vget /unrealcv/version`
    - Add :code:`vset /action/eyes_distance`
    - Fix :code:`vget /camera/[id]/location` to support multiple cameras
    - Update test code
- v0.3.1 : Fix GWorld issue

v0.3.0 - Stability improvement
==============================

- Add support for Unreal 4.13, 4.14
- Stability improvement, fix crash caused by the usage of GWorld
- Fix some incorrect ground truth, blueprint actor not correctly displayed.
- Add playground project
- Add docs to docs.unrealcv.org
- Add API documentation created by doxygen
- Fix an issue that prevents the packaging of games.
- Add vrun command to exec UE4 built-in command

API update:

- :code:`vrun [built-in command]`
- :code:`vset /camera/[id]/moveto [x] [y] [z]` # With collision enabled


v0.2.0 - First public release
=============================

Features

- Add communication to UE4Editor and a compiled game
- Add Python and MATLAB client to communicate with UnrealCV server
- Add ground truth extraction, include: depth, object-mask, surface normal
- Add support for Linux, Win and Mac

Initial API, see more details in :doc:`the command list </reference/commands>`

- :code:`vget /objects`
- :code:`vget /object/[obj_name]/color`
- :code:`vset /object/[obj_name]/color [r] [g] [b]`
- :code:`vget /camera/[id]/location`
- :code:`vget /camera/[id]/rotation`
- :code:`vset /camera/[id]/location [x] [y] [z]`
- :code:`vset /camera/[id]/rotation [pitch] [yaw] [roll]`
- :code:`vget /camera/[id]/[viewmode]`
- :code:`vget /camera/[id]/[viewmode] [filename]`
- :code:`vset /viewmode [viewmode]`
- :code:`vget /viewmode`
- :code:`vget /unrealcv/status`
- :code:`vget /unrealcv/help`

The upcoming release will follow the concept of `Semantic Versioning <http://semver.org/>`__

=========
CHANGELOG
=========

Development branch
==================

- head (changes not tagged yet):
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

Python API Reference
====================

This document provides detailed API reference for UnrealCV Python client.

Basic Usage
-----------

.. automodule:: unrealcv
   :members:
   :show-inheritance:

UnrealCV Client
---------------

The client module exposes the main `Client` interface used to connect to UnrealCV.

UnrealCV API
------------

The UnrealCV API provides high-level functions to interact with Unreal Engine.

The reference below is generated from the local ``client/python`` package used by this branch, so newly added public methods are included automatically. Most command-building methods accept ``return_cmd=True``; this returns the UnrealCV command without sending it and is useful for inspection, batching, and tests.

Current 5.2 additions include positioned object spawning and bone queries through :meth:`unrealcv.api.UnrealCv_API.set_new_obj`, :meth:`unrealcv.api.UnrealCv_API.spawn_object_from_path`, and :meth:`unrealcv.api.UnrealCv_API.get_obj_bones`.

The recording, annotation, pak, panoramic, scene occupancy, and CID helpers are also documented below. They target the extended UnrealCV+ server surface and require a server build that registers their corresponding commands; the base command inventory is listed in :doc:`commands`.

.. automodule:: unrealcv.api
    :members:
    :show-inheritance:

Environment Launcher
--------------------

The launcher module provides utilities to start and manage Unreal Engine environments.

.. automodule:: unrealcv.launcher
    :members:
    :show-inheritance:

Automation Tools
----------------

Tools for building plugins and packaging model zoo binaries.

.. automodule:: unrealcv.automation
    :members:
    :show-inheritance:

Groom Wind Helpers
------------------

.. automodule:: unrealcv.groom_wind
    :members:

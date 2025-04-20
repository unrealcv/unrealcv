Python API Reference
===================

This document provides detailed API reference for UnrealCV Python client.

Basic Usage
----------

Basic Usage of UnrealCV Client
------------------
.. automodule:: unrealcv
   :members:
   :undoc-members:
   :show-inheritance:

UnrealCV Client
--------------

.. autoclass:: Client
    :members:
    :undoc-members:
    :show-inheritance:

UnrealCV API
-----------

UE Binary Launcher
------------------
The unrealcv.launcher module provides a simple way to launch UE Binary with UnrealCV plugin enabled.

.. automodule:: unrealcv.launcher

.. autoclass:: RunUnreal
    :members:

Utility Functions to call UnrealCV commands
------------------
The unrealcv.api module provides some utility functions to help using UnrealCV.

.. automodule:: unrealcv.api

.. autoclass:: UnrealCv_API
    :members:

Automation tools
------------------
The unrealcv.automation module to help building the plugin, packaging model zoo binaries, etc.

.. automodule:: unrealcv.automation
    :members:
    :undoc-members:
    :show-inheritance:

.. autofunction:: UE4Binary

.. autoclass:: UE4Automation
    :members:
    :undoc-members:
    :show-inheritance:

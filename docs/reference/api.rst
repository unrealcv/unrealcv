API
===
The C++ code of the plugin is documented with doxygen. You can generate the API document with :code:`cd docs/doxygen; doxygen`. An online version is hosted in `here <https://codedocs.xyz/unrealcv/unrealcv/>`__.

.. _python_client:

Python
------

The python client

.. automodule:: unrealcv

.. autoclass:: Client
    :members:

.. autoclass:: BaseClient
    :members:

.. _matlab_client:

MATLAB
------
The matlab client

Core
----

`FUE4CVServer` uses `FCommandDispatcher` to execute commands. All CommandHandlers are registered.

.. doxygenclass:: FUE4CVServer
    :members:

.. doxygenclass:: UNetworkManager

.. doxygenclass:: FExecStatus

.. doxygenclass:: FPromise

.. doxygenclass:: FCommandDispatcher

.. doxygenclass:: FAsyncWatcher

The execution result of a command

Basic
-----
.. doxygenclass:: AUE4CVPawn

.. doxygenclass:: AUE4CVCharacter

.. doxygenclass:: AUE4CVGameMode

.. doxygenclass:: FServerConfig

API
---
.. doxygenclass:: FCommandDispatcher

.. doxygenclass:: FCommandHandler

.. doxygenclass:: FActionCommandHandler

.. doxygenclass:: FAliasCommandHandler

.. doxygenclass:: FCameraCommandHandler

.. doxygenclass:: FObjectCommandHandler

.. doxygenclass:: FPluginCommandHandler

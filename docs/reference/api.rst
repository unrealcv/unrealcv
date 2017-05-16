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
.. doxygenclass:: FUE4CVServer
    :members:

.. doxygenclass:: UNetworkManager

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

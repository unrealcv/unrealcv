Command Dispatch Flow
=====================

This page summarizes how a TCP request reaches an UnrealCV command handler.

Request Flow
------------

.. code-block:: text

    TCP client
        |
        |  "vget /camera/0/location"
        v
    UnrealCV server
        |
        v
    Command dispatcher
        |
        +-- Parse verb and URI
        +-- Match registered template
        +-- Convert arguments
        |
        v
    Command handler
        |
        v
    FExecStatus
        |
        +-- Success payload
        +-- Error message
        v
    TCP response

Registration Flow
-----------------

.. code-block:: text

    Handler construction
        |
        v
    RegisterCommands()
        |
        v
    BindCommand(template, delegate, help text)
        |
        v
    Dispatcher registry

Command Template Example
------------------------

.. code-block:: cpp

    CommandDispatcher->BindCommand(
        "vget /camera/[uint]/location",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLocation),
        "Get camera location"
    );

Runtime Sequence
----------------

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Stage
     - Responsibility
   * - Receive
     - Read one command request from the client connection.
   * - Match
     - Resolve the request against registered URI templates.
   * - Parse
     - Convert path arguments to the types declared by the template.
   * - Execute
     - Invoke the bound handler delegate.
   * - Return
     - Serialize ``FExecStatus`` as the TCP response.

Error Flow
----------

.. code-block:: text

    No matching template ------> "Can not find a handler for URI ..."
    Invalid arguments ---------> Handler usage or conversion error
    Runtime operation fails ---> FExecStatus::Error(...)
    Operation succeeds --------> FExecStatus::OK(...)

Related Pages
-------------

- :doc:`../../reference/commands` for the base command reference.
- :doc:`../reference/commands` for UnrealCV Dev For UnrealZoo commands.
- :doc:`../runtime-mcp` for MCP exposure of runtime capabilities.

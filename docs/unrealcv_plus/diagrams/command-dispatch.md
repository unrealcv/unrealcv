Command Dispatch Flow
==================

TCP command processing flow from connection to handler execution.

Overview
--------

::

   External Client                 UnrealCV Server
   ───────────────                 ───────────────

   ┌─────────┐                     ┌─────────────────────────┐
   │ Python  │ ─── TCP Request ──▶ │ FUnrealcvServer        │
   │ Client  │                     │ - TCP Connection       │
   └─────────┘                     │ - Command Dispatcher   │
                                    └─────────────────────────┘
                                                  │
                                                  ▼
                                    ┌─────────────────────────┐
                                    │ CommandDispatcher       │
                                    │ - Parse command         │
                                    │ - Route to handler     │
                                    └─────────────────────────┘
                                                  │
                    ┌─────────────────────────────┼─────────────────────────────┐
                    ▼                             ▼                             ▼
            ┌───────────────┐             ┌───────────────┐             ┌───────────────┐
            │ CameraHandler │             │ ObjectHandler │             │ OtherHandlers │
            └───────────────┘             └───────────────┘             └───────────────┘
                    │                             │                             │
                    └─────────────────────────────┼─────────────────────────────┘
                                                  │
                                                  ▼
                                    ┌─────────────────────────┐
                                    │ FExecStatus           │
                                    │ - OK / Error          │
                                    │ - Result string       │
                                    └─────────────────────────┘
                                                  │
                                                  ▼
                                    ┌─────────────────────────┐
                                    │ Return to client       │
                                    └─────────────────────────┘

Command Format
-------------

**vget** (Query/Retrieve):

::

   vget /handler/subcommand [args]

   Example: vget /camera/0/lit
   Example: vget /cameras

**vset** (Action/Execute):

::

   vset /handler/subcommand [args]

   Example: vset /object/actor/visible 0
   Example: vset /action/pause 1

**vbp** (Blueprint Bridge):

::

   vbp /object_name/function param1=value1 param2=value2

   Example: vbp /camera/0/startrecording type=normal

Dispatch Flow
------------

::

   ┌──────────────────────────────────────────────────────────────────┐
   │                      CommandDispatcher                             │
   └──────────────────────────────────────────────────────────────────┘

   ┌──────────────────────────────────────────────────────────────────┐
   │ Step 1: Parse                                                    │
   └──────────────────────────────────────────────────────────────────┘

   Input: "vget /camera/0/lit"

   Parse:
   - Command type: vget / vset / vbp
   - Handler: camera
   - Subcommand: 0/lit
   - Args: []

   ┌──────────────────────────────────────────────────────────────────┐
   │ Step 2: Route                                                    │
   └──────────────────────────────────────────────────────────────────┘

   Lookup: /camera/* ──▶ FCameraHandler

   ┌──────────────────────────────────────────────────────────────────┐
   │ Step 3: Execute                                                  │
   └──────────────────────────────────────────────────────────────────┘

   FCameraHandler::GetCameraLit(Args)

   Handler:
   1. Validate arguments
   2. Execute logic
   3. Return FExecStatus

   ┌──────────────────────────────────────────────────────────────────┐
   │ Step 4: Respond                                                  │
   └──────────────────────────────────────────────────────────────────┘

   FExecStatus::OK(image_data)

   Response: "ok [base64_data]"

Handler Registration Pattern
---------------------------

::

   void FCameraHandler::RegisterCommands()
   {
       CommandDispatcher->BindCommand(
           "vget /camera/[int]/lit [str]",
           FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLit),
           "Get the lit image from camera"
       );

       CommandDispatcher->BindCommand(
           "vget /cameras",
           FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraList),
           "List all cameras"
       );
   }

Pattern: "command_pattern" + Delegate + HelpString

Command Patterns:

+---------------------------+----------------------------------------+
| Pattern                   | Description                            |
+---------------------------+----------------------------------------+
| ``/camera/[int]/lit``     | Integer parameter                      |
| ``/object/[str]/location``| String parameter                       |
| ``/camera/*``             | Wildcard (any subcommand)              |
+---------------------------+----------------------------------------+

vbp Special Case
---------------

::

   ┌──────────────────────────────────────────────────────────────────┐
   │                        vbp Command Flow                           │
   └──────────────────────────────────────────────────────────────────┘

   Input: "vbp /camera/0/startrecording type=normal"

   ┌─────────────────┐          ┌─────────────────┐
   │ Parse object   │   ───▶   │ Find actor by  │
   │ path           │          │ name            │
   └─────────────────┘          └─────────────────┘
                                        │
                                        ▼
                                ┌─────────────────┐
                                │ Call Blueprint  │
                                │ function        │
                                │ dynamically     │
                                └─────────────────┘
                                        │
                                        ▼
                                ┌─────────────────┐
                                │ Return result   │
                                └─────────────────┘

Error Handling
-------------

::

   ┌──────────────────────────────────────────────────────────────────┐
   │                      FExecStatus Types                           │
   └──────────────────────────────────────────────────────────────────┘

   ┌─────────────────┐          ┌─────────────────┐
   │ OK              │          │ Error           │
   │ - Result data   │          │ - Error message │
   │ - Optional      │          │ - Always error  │
   └─────────────────┘          └─────────────────┘
          │                            │
          ▼                            ▼
   "ok data"                  "error: message"

Response Examples
----------------

::

   Success:
   ───────
   Request:  vget /cameras
   Response: ok [0, 1, 2]

   Request:  vget /camera/0/lit
   Response: ok [base64_encoded_image_data]

   Error:
   ──────
   Request:  vget /camera/999/lit
   Response: error: Camera 999 not found

   Request:  vset /object/missing/location 100 200
   Response: error: Object 'missing' not found

UnrealCV Dev For UnrealZoo
==========================

UnrealCV Dev For UnrealZoo is the continuously developed and tested feature
surface used by UnrealZoo. These capabilities are currently available in
UnrealZoo builds before they are promoted into the open-source UnrealCV plugin.
Use an UnrealZoo environment when following these pages.

This is an availability label, not a second open-source plugin distribution.
The open-source command contract remains :doc:`../reference/commands`; the
development command inventory is documented separately so clients can detect
and handle the difference explicitly.

The development features are tested in UnrealZoo first. General-purpose parts
that mature without project-specific dependencies may later move into the
open-source plugin. The Runtime MCP server implementation is not currently part
of this repository; public clients, examples, and agent skills are maintained
in the `UnrealCV Runtime MCP repository
<https://github.com/lizi-Margin/unrealcv-runtime-mcp>`_. This public staging
repository is intended to move to the ``unrealcv`` organization.

Start with :doc:`runtime-mcp` for the agent workflow or :doc:`overview` for the
current development feature summary.

.. toctree::
    :maxdepth: 2
    :caption: Overview
    :titlesonly:

    overview
    runtime-mcp

.. toctree::
    :maxdepth: 2
    :caption: Development Commands
    :titlesonly:

    reference/light-commands
    reference/pak-commands
    reference/sensor-data-formats
    reference/commands

.. toctree::
    :maxdepth: 2
    :caption: Architecture
    :titlesonly:

    architecture/sensor-system
    architecture/annotation-system

.. toctree::
    :maxdepth: 2
    :caption: Workflows
    :titlesonly:

    tutorials/pak-workflow
    integration/gym-unrealcv
    migration/camera-id-format

.. toctree::
    :maxdepth: 1
    :caption: Diagrams
    :titlesonly:

    diagrams/annotation-decision
    diagrams/command-dispatch

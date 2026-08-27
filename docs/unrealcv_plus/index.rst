UnrealCV Dev for `UnrealZoo <https://github.com/UnrealZoo>`_
==============================================================

UnrealCV Dev for `UnrealZoo <https://github.com/UnrealZoo>`_ is the continuously
developed and tested feature surface used by `UnrealZoo <https://github.com/UnrealZoo>`_.
These capabilities are currently available in `UnrealZoo <https://github.com/UnrealZoo>`_
builds before they are promoted into the open-source UnrealCV plugin. Use an
`UnrealZoo <https://github.com/UnrealZoo>`_ environment when following these pages.

This is an availability label, not a second open-source plugin distribution.
The open-source command contract remains :doc:`../reference/commands`; the
development command inventory is documented separately so clients can detect
and handle the difference explicitly.

The development features are tested in `UnrealZoo <https://github.com/UnrealZoo>`_ first. General-purpose parts
that mature without project-specific dependencies may later move into the
open-source plugin. The Runtime MCP server implementation is not currently part
of this repository; public clients, examples, and agent skills are maintained
in the `UnrealCV Runtime MCP repository
<https://github.com/lizi-Margin/unrealcv-runtime-mcp>`_. This public staging
repository is intended to move to the ``unrealcv`` organization.

Start with :doc:`overview` for the current development feature summary.

.. toctree::
    :maxdepth: 2
    :caption: Overview
    :titlesonly:

    overview

.. toctree::
    :maxdepth: 2
    :caption: Development Commands
    :titlesonly:

    reference/light-commands
    reference/pak-commands
    Scene perception <reference/scene-perception>
    reference/panoramic-camera
    reference/video-recording-pipeline
    reference/annotation-system
    reference/object-spawning-from-path
    reference/mqrc-rendering
    reference/runtime-reflection

.. toctree::
    :maxdepth: 2
    :caption: Architecture
    :titlesonly:

    migration/camera-id-format

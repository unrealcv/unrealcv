UnrealCV+
=========

This section contains features and implementation notes from the extended
UnrealCV+ codebase. These pages are intentionally separate from the open-source
UnrealCV command reference because the documented server capabilities are not
available in every UnrealCV build.

Start with :doc:`overview` for a feature summary. The command pages below are
the authoritative UnrealCV+ command documentation; the base open-source command
reference remains at :doc:`../reference/commands`.

.. toctree::
    :maxdepth: 2
    :caption: Overview

    overview
    runtime-mcp

.. toctree::
    :maxdepth: 2
    :caption: UnrealCV+ Commands

    reference/light-commands
    reference/pak-commands
    reference/sensor-data-formats
    reference/commands

.. toctree::
    :maxdepth: 2
    :caption: Architecture

    architecture/sensor-system
    architecture/annotation-system

.. toctree::
    :maxdepth: 2
    :caption: Workflows

    tutorials/pak-workflow
    integration/gym-unrealcv
    migration/camera-id-format

.. toctree::
    :maxdepth: 1
    :caption: Diagrams

    diagrams/annotation-decision
    diagrams/command-dispatch

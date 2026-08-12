Pak File Workflow Tutorial
==========================

This guide describes the runtime pak workflow supported by UnrealCV+:
mount a cooked pak, inspect its contents, scan its package paths, and load an
asset. The commands and Blueprint signatures below match the current
UnrealCV+ implementation.

Workflow
--------

::

    Cook pak -> Mount pak -> Inspect paths -> Scan mount point -> Load asset

Requirements
------------

- The pak must contain content cooked for the running platform and engine build.
- The Unreal process must be able to read the pak file path.
- Asset paths must use Unreal package names such as ``/Game/Props/BP_Chair``.
- A higher ``PakOrder`` gives the pak a later loading priority.

Mount a Pak
-----------

TCP command:

.. code-block:: console

    vset /pak/mount D:/Paks/Assets_Win64.pak 0
    Mounted: D:/Paks/Assets_Win64.pak (Order: 0)

Equivalent Blueprint-callable C++ API:

.. code-block:: cpp

    const bool bMounted = UPakMountBPLib::MountPakFile(
        TEXT("D:/Paks/Assets_Win64.pak"),
        0 // PakOrder
    );

List and Check Mounted Paks
---------------------------

List mounted pak files. The returned indices are one-based and may be used by
commands that accept ``PakFilePath|PakIndex``.

.. code-block:: console

    vget /pak/mounted
    1. D:/Paks/Assets_Win64.pak

Check by path or logical index:

.. code-block:: console

    vget /pak/ismounted 1
    1

    vget /pak/ismounted D:/Paks/Assets_Win64.pak
    1

Inspect Pak Contents
--------------------

List every file recorded in the pak:

.. code-block:: console

    vget /pak/files 1

List asset package paths inferred from the pak:

.. code-block:: console

    vget /pak/assets_in_pak 1

Inspect the package paths registered from the pak mount point:

.. code-block:: console

    vget /pak/registered_paths 1
    vget /pak/registered_status 1

The status command returns tab-separated rows containing the package path,
file-existence status, loadability status, and an optional resolved filename.

Scan Mounted Assets
-------------------

Scan the Asset Registry after mounting. The first argument is a virtual mount
point, not a filesystem directory.

.. code-block:: console

    vset /pak/scan /Game/Assets 1
    Scanned: /Game/Assets

Equivalent C++ API:

.. code-block:: cpp

    UPakMountBPLib::ScanMountedAssets(
        TEXT("/Game/Assets"),
        true // Force rescan
    );

List Assets in a Package Path
-----------------------------

.. code-block:: console

    vget /pak/assets /Game/Assets

Equivalent C++ API with an optional class filter:

.. code-block:: cpp

    const TArray<FString> Meshes = UPakMountBPLib::GetAllAssetsInPath(
        TEXT("/Game/Assets"),
        UStaticMesh::StaticClass()
    );

Load an Asset
-------------

.. code-block:: console

    vget /pak/load /Game/Assets/BP_Chair.BP_Chair
    Loaded: /Game/Assets/BP_Chair.BP_Chair (Class: Blueprint)

Equivalent C++ API:

.. code-block:: cpp

    UObject* LoadedObject = UPakMountBPLib::LoadAssetFromPak(
        TEXT("/Game/Assets/BP_Chair.BP_Chair"),
        nullptr
    );

The asset path must match an object path reported by the Asset Registry. A
package-only path may not be sufficient for every asset type.

Complete TCP Example
--------------------

.. code-block:: console

    vset /pak/mount D:/Paks/Assets_Win64.pak 0
    vget /pak/mounted
    vget /pak/assets_in_pak 1
    vget /pak/registered_status 1
    vset /pak/scan /Game/Assets 1
    vget /pak/assets /Game/Assets
    vget /pak/load /Game/Assets/BP_Chair.BP_Chair

Unmount a Pak
-------------

Unmount by path or logical index:

.. code-block:: console

    vset /pak/unmount 1

Equivalent C++ API:

.. code-block:: cpp

    const bool bUnmounted = UPakMountBPLib::UnmountPakFile(
        TEXT("D:/Paks/Assets_Win64.pak")
    );

Common Problems
---------------

Pak fails to mount
    Verify that the file exists, was cooked for the current platform, and is
    readable by the Unreal process.

Assets are missing after mounting
    Run ``vget /pak/registered_status`` and then scan the reported mount point
    with ``vset /pak/scan``.

Asset fails to load
    Use an exact object path returned by ``vget /pak/assets`` and verify that
    the asset class is available in the running build.

See Also
--------

- :doc:`../reference/pak-commands` for the complete command reference.
- :doc:`../overview` for the UnrealCV+ feature summary.

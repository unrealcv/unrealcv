Pak File Management System
==========================

This section documents runtime pak file mounting and asset loading capabilities (UE5.2+).

Overview
--------

UnrealCV supports dynamic loading of pak files at runtime, enabling:

- **On-demand asset loading**: Load assets without restarting the game
- **DLC-like functionality**: Package content in separate pak files
- **Asset pooling**: Register pak assets to AssetPoolManager for random selection
- **Hot-swappable content**: Mount/unmount paks during gameplay

.. note::
   Pak file management requires cooked content. Pak files must be created
   using UE5's packaging system with appropriate compression settings.

Pak File Requirements
---------------------

1. **Cooked Content**: Pak files must be cooked for the target platform
2. **Mount Point**: Pak files need a valid mount point (e.g., ``/Game/``)
3. **Loading Order**: Use ``PakOrder`` for dependency management

Creating Pak Files
~~~~~~~~~~~~~~~~~~

In UE5 Editor:

1. Package your project: ``Platforms > Windows > Package Project``
2. Or use UnrealPak tool directly::

    UnrealPak.exe MyProject_Paks/MyPak.pak /Create=/Game/MyFolder

3. Copy pak files to a location accessible at runtime

.. _pak-tcp-commands:

TCP Commands
------------

Pak Management
~~~~~~~~~~~~~~

vset /pak/mount [PakFilePath] [PakOrder=0]
    Mount a pak file at runtime.

    Arguments:
    - ``PakFilePath``: Full path to the pak file
    - ``PakOrder``: Loading order (higher = loaded later)

    Example: ``vset /pak/mount D:/MyContent/MyAssets.pak 0``

    Returns: ``Mounted: D:/MyContent/MyAssets.pak (Order: 0)``

vset /pak/unmount [PakFilePath]
    Unmount a previously mounted pak file.

    Example: ``vset /pak/unmount D:/MyContent/MyAssets.pak``

vget /pak/mounted
    Get list of all currently mounted pak files.

    Returns: Newline-separated numbered list of pak file paths, starting at 1

    Example output::

        1. D:/MyContent/MyAssets.pak
        2. D:/MyContent/Shared.pak

vget /pak/ismounted [PakFilePath|PakIndex]
    Check if a pak file is currently mounted.

    You can pass either a full pak file path or the logical index shown by
    ``vget /pak/mounted``.

    Returns: ``1`` if mounted, ``0`` if not

Pak Asset Operations
~~~~~~~~~~~~~~~~~~~~

vset /pak/scan [MountPoint] [bForceRescan=1]
    Scan assets in a mounted pak file.

    Arguments:
    - ``MountPoint``: Mount point path (e.g., ``/Game/``)
    - ``bForceRescan``: Force rescan even if already scanned

    Example: ``vset /pak/scan /Game/ 1``

vget /pak/load [AssetPath]
    Load a specific asset from a mounted pak.

    Arguments:
    - ``AssetPath``: Full asset path (e.g., ``/Game/Assets/Car/BP_Car.uasset``)

    Returns: ``Loaded: /Game/Assets/Car/BP_Car.uasset (Class: Blueprint)``

vget /pak/assets [PackagePath]
    Get all assets in a package path.

    Arguments:
    - ``PackagePath``: Package path with wildcard (e.g., ``/Game/Assets/*``)

    Returns: Newline-separated list of asset paths

vget /pak/registered_paths [PakFilePath|PakIndex]
    Get all package paths that can be resolved from a pak file using its mount point.

    Arguments:
    - ``PakFilePath``: Full path to the pak file
    - ``PakIndex``: Logical index from ``vget /pak/mounted`` (1-based)

    Returns: Newline-separated list of long package names

vget /pak/registered_status [PakFilePath|PakIndex]
    Get all package paths resolved from a pak file along with existence status.

    Arguments:
    - ``PakFilePath``: Full path to the pak file
    - ``PakIndex``: Logical index from ``vget /pak/mounted`` (1-based)

    Returns: Newline-separated rows in the form ``PackagePath<TAB>found|missing<TAB>loadable|not_loadable[<TAB>Filename]``

Compatibility Scan Alias
~~~~~~~~~~~~~~~~~~~~~~~~

vset /pak/register [PackagePath] [Category]
    Compatibility alias that force-scans ``PackagePath`` in the Asset Registry.

    Arguments:
    - ``PackagePath``: Package path to scan (for example, ``/Game/Assets``)
    - ``Category``: Required by the command template but currently ignored

    Example: ``vset /pak/register /Game/Assets compatibility``

    The current implementation returns ``Scanned assets from '<PackagePath>'``.
    It does not register an AssetPool category. Prefer ``vset /pak/scan`` in new
    integrations.

.. _pak-blueprint-api:

Blueprint API
-------------

PakMountBPLib provides Blueprint-accessible functions for pak management.

Mounting Functions
~~~~~~~~~~~~~~~~~~

``bool MountPakFile(const FString& PakFilePath, int32 PakOrder = 0)``

   Mount a pak file from Blueprint.

   :param PakFilePath: Full path to the pak file (string)
   :param PakOrder: Loading order integer (default: 0)
   :return: true if mounted successfully

``bool UnmountPakFile(const FString& PakFilePath)``

   Unmount a previously mounted pak file.

   :param PakFilePath: Path of the pak file to unmount, or a mounted index
   :return: true if unmounted successfully

``TArray<FString> GetMountedPakFiles()``

   Get list of all currently mounted pak file paths.

   :return: Array of pak file paths

``bool IsPakFileMounted(const FString& PakFilePath)``

   Check if a specific pak file is mounted.

   :param PakFilePath: Path to check, or a mounted index
   :return: true if mounted

Asset Loading Functions
~~~~~~~~~~~~~~~~~~~~~~~

``void ScanMountedAssets(const FString& MountPoint, bool bForceRescan = true)``

   Scan and register asset registry for a mount point.

   :param MountPoint: Mount point path (e.g., ``/Game/``)
   :param bForceRescan: Force rescan even if already scanned

``TArray<FString> GetAllAssetsInPath(const FString& PackagePath, UClass* AssetClass = nullptr)``

   Get all assets in a package path.

   :param PackagePath: Package path with optional wildcard
   :param AssetClass: Optional class filter (e.g., ``UBlueprint::StaticClass()``)
   :return: Array of asset paths

``TArray<FString> GetRegisteredPackagePathsInPakFile(const FString& PakFilePath)``

   Get all long package names that can be resolved from a pak file.

   :param PakFilePath: Full path to the pak file, or a mounted index
   :return: Array of long package names

``TArray<FString> GetRegisteredPackageStatusInPakFile(const FString& PakFilePath)``

   Get all resolved package paths from a pak file with existence status.

   :param PakFilePath: Full path to the pak file, or a mounted index
   :return: Array of status rows

``UObject* LoadAssetFromPak(const FString& AssetPath, UClass* AssetClass)``

   Load a specific asset from pak.

   :param AssetPath: Full asset path
   :param AssetClass: Expected class (can be nullptr)
   :return: Loaded UObject or nullptr

.. _pak-workflow-example:

Workflow Example
----------------

**Step 1: Mount pak file**

::

    vset /pak/mount D:/Content/MyAssets.pak 0

    # Check if mounted
    vget /pak/mounted

**Step 2: Scan for assets**

::

    vset /pak/scan /Game/ 1

    # List all assets
    vget /pak/assets /Game/MyAssets/*

**Step 3: Register to asset pool**

::

    vset /pak/register /Game/MyAssets/* Foreground_Human

**Step 4: Use in scene composition**

Now assets are available for random selection via SceneCompositionBPLib::

    SpawnRandomForeground(Category="Foreground_Human")

Common Issues
-------------

**Asset not found after mounting**

- Ensure mount point path is correct
- Call ``vset /pak/scan [MountPoint] 1`` after mounting
- Verify pak file was cooked for the correct platform

**LoadAsset returns nullptr**

- Check that the asset path is exactly correct (case-sensitive)
- Verify the asset class matches expected type
- Ensure the asset is in a mounted pak file

**Registration to asset pool fails**

- Confirm the category name is valid
- Check that assets were scanned before registration
- Verify the package path has wildcard (*) for multiple assets

See Also
--------

- :doc:`../overview` - UnrealCV Dev For `UnrealZoo <https://github.com/UnrealZoo>`_ feature overview
- :doc:`../../tutorials/pak-workflow` - Pak workflow tutorial

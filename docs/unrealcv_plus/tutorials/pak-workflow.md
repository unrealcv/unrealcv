Pak File Workflow Tutorial
=========================

This guide explains how to use UnrealCV's Pak file management system
for dynamic asset loading at runtime.

Overview
--------

Pak files are UE's archive format for shipping assets separately from
the main application. UnrealCV's Pak system enables:

- Runtime asset loading without recompilation
- Dynamic asset pools for dataset generation
- Modular content management
- On-demand asset registration

**Key Components:**

- ``UPakMountBPLib`` - Blueprint library for Pak operations
- ``FPakHandler`` - TCP command handler
- ``FAssetPoolManager`` - Asset registration and retrieval

Workflow
--------

::

   Pak File          Mount           Scan           Register          Asset Pool
   ──────── ────── ──────── ────── ──────── ────── ──────────── ─────────────
   Create  ─────> Mount ─────> Scan ─────> Register ─────> Available for
   (.pak)          Pak            Assets          to Pool         Spawning

1. **Create** Pak files via UE Editor packaging
2. **Mount** Pak at runtime
3. **Scan** for assets in the mounted Pak
4. **Register** to AssetPoolManager
5. **Spawn** assets via SceneCompositionBPLib

Creating Pak Files
-----------------

**Editor Method:**

1. Open Package Project dialog: File > Package Project > Pak
2. Select output directory
3. Name the Pak file (e.g., ``Assets.pak``)
4. Result: ``Assets_Win64.pak``

**Python/Automation Method:**

Use UE's automation tools for batch Pak creation.

Mounting Pak Files
------------------

**Blueprint API:**

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static bool MountPakFile(const FString& PakFilePath, int32 PakOrder = 0);

**Example:**

.. code-block:: cpp

   bool bMounted = UPakMountBPLib::MountPakFile(
       TEXT("D:/Paks/Assets_Win64.pak"),
       0  // PakOrder for loading priority
   );

**TCP Command:**

.. code-block:: bash

   vset /pak/mount D:/Paks/Assets_Win64.pak 0
   ok Mounted: D:/Paks/Assets_Win64.pak (Order: 0)

**Parameters:**

- ``PakFilePath`` - Absolute path to the Pak file
- ``PakOrder`` - Loading priority (higher = loaded later, overrides)

Unmounting Pak Files
--------------------

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static bool UnmountPakFile(const FString& PakFilePath);

**TCP:**

.. code-block:: bash

   vset /pak/unmount D:/Paks/Assets_Win64.pak
   ok Unmounted: D:/Paks/Assets_Win64.pak

Scanning Assets
--------------

After mounting, scan to discover available assets:

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static void ScanMountedAssets(const FString& MountPoint, bool bForceRescan = true);

**TCP:**

.. code-block:: bash

   vset /pak/scan /Game/Assets 1
   ok Scanned: /Game/Assets

**Parameters:**

- ``MountPoint`` - Virtual path prefix (e.g., ``/Game/Assets``)
- ``bForceRescan`` - Force re-scanning cached assets

Listing Assets
-------------

**Get All Assets in Path:**

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static TArray<FString> GetAllAssetsInPath(const FString& PackagePath, UClass* AssetClass = nullptr);

**TCP:**

.. code-block:: bash

   vget /pak/assets /Game/Assets
   ok /Game/Assets/Character1
   /Game/Assets/Character2
   /Game/Assets/Prop1

**Filter by Class:**

.. code-block:: cpp

   // Get only StaticMesh assets
   TArray<FString> Meshes = UPakMountBPLib::GetAllAssetsInPath(
       TEXT("/Game/Assets"),
       UStaticMesh::StaticClass()
   );

Loading Assets
--------------

**Load Single Asset:**

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static UObject* LoadAssetFromPak(const FString& AssetPath, UClass* AssetClass);

**TCP:**

.. code-block:: bash

   vget /pak/load /Game/Assets/Character1
   ok Loaded: /Game/Assets/Character1 (Class: AActor)

**Example - Spawn Loaded Actor:**

.. code-block:: cpp

   UObject* LoadedObj = UPakMountBPLib::LoadAssetFromPak(
       TEXT("/Game/Assets/Character1"),
       AActor::StaticClass()
   );

   if (LoadedObj)
   {
       AActor* NewActor = GetWorld()->SpawnActor(LoadedObj->GetClass());
   }

Registering to Asset Pool
--------------------------

Assets can be registered to the AssetPoolManager for random selection:

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static bool RegisterAssetsToAssetPool(const FString& PackagePath, const FString& Category);

**TCP:**

.. code-block:: bash

   vset /pak/register /Game/Assets/ foreground
   ok Registered assets from '/Game/Assets/' to category 'foreground'

**Parameters:**

- ``PackagePath`` - Path to scan (e.g., ``/Game/Assets``)
- ``Category`` - Asset pool category (``foreground``, ``occluder``, ``scene``)

**Asset Pool Categories:**

+------------------+---------------------------------------------------+
| Category         | Description                                       |
+------------------+---------------------------------------------------+
| foreground       | Primary objects for dataset                       |
| occluder         | Objects that can occlude foreground               |
| scene            | Background environment assets                     |
| prop             | Small props and details                           |
| character        | Human/creature models                             |
+------------------+---------------------------------------------------+

Checking Mount Status
--------------------

**Is Pak Mounted:**

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static bool IsPakFileMounted(const FString& PakFilePath);

**TCP:**

.. code-block:: bash

   vget /pak/ismounted D:/Paks/Assets_Win64.pak
   ok 1

**List All Mounted:**

.. code-block:: cpp

   UFUNCTION(BlueprintCallable, Category = "UnrealCV|PakMount")
   static TArray<FString> GetMountedPakFiles();

**TCP:**

.. code-block:: bash

   vget /pak/mounted
   ok D:/Paks/Assets1_Win64.pak
   D:/Paks/Assets2_Win64.pak

Complete Workflow Example
------------------------

**Step 1: Mount Pak**

.. code-block:: cpp

   void LoadDatasetAssets()
   {
       // Mount asset Pak
       bool bSuccess = UPakMountBPLib::MountPakFile(
           TEXT("D:/Datasets/HumanCharacters_Win64.pak"),
           0
       );

       if (!bSuccess)
       {
           UE_LOG(LogTemp, Error, TEXT("Failed to mount Pak"));
           return;
       }
   }

**Step 2: Scan and Register**

.. code-block:: cpp

   // Scan for all human character assets
   UPakMountBPLib::ScanMountedAssets(TEXT("/Game/Characters/Humans"));

   // Register to asset pool
   UPakMountBPLib::RegisterAssetsToAssetPool(
       TEXT("/Game/Characters/Humans"),
       TEXT("foreground")
   );

**Step 3: Use in Scene Generation**

.. code-block:: cpp

   // Later, spawn random from pool
   FAssetHandle AssetHandle;
   bool bFound = USceneCompositionBPLib::SpawnRandomForeground(
       GetWorld(),
       TEXT("foreground"),
       FVector(0, 0, 0),
       AssetHandle
   );

Common Issues
-------------

**Pak won't mount:**
- Verify file path exists and is accessible
- Check Pak was created for correct platform
- Ensure no other process has file locked

**Assets not found after scan:**
- Confirm MountPoint matches Pak's virtual paths
- Try bForceRescan = true
- Check Pak was properly created with assets

**Registration fails:**
- Verify AssetPoolManager is initialized
- Check category name is valid
- Ensure scanned path contains assets

**LoadAsset returns nullptr:**
- Verify asset path is correct
- Check asset wasn't already loaded
- Ensure class type matches asset type

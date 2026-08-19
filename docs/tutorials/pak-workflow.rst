Import Custom Assets with an Unreal Pak
=======================================

This guide explains how to package custom Unreal Engine assets into a standard
``.pak`` file and make them available to a packaged binary. The workflow uses
Unreal Engine's normal PAK and Asset Manager features and follows the general
approach described by `SimWorld <https://simworld.readthedocs.io/en/latest/customization/make_your_own_pak.html#>`_.

Quick Start
-----------

1. Place custom assets in a UE project, for example ``YourProject/Content/MyAssets/``.
2. Create a PAK file from those assets.
3. Copy the PAK into the packaged application's ``Content/Paks/`` directory.
4. Start the packaged binary. Applications that support runtime PAK discovery
   can mount and scan the assets during startup.

Prerequisites
-------------

- Unreal Engine 5.6 or a compatible engine version.
- A packaged binary whose runtime supports loading external PAK files.
- Basic familiarity with Unreal Engine asset creation and packaging.

Step 1. Create a PAK File
-------------------------

Use Unreal Engine's standard PAK loading workflow. See the
`official Epic tutorial <https://dev.epicgames.com/community/learning/tutorials/7Bj8/unreal-engine-example-project-loading-pak-files-at-runtime>`_.
The general process is:

1. Create a normal Unreal Engine project.
2. Import your custom assets.
3. Cook the assets.
4. Package them into a ``.pak`` file.

Users only need to package their assets into a PAK file and place it in the
packaged binary's PAK directory. The runtime-specific mount and scan behavior
depends on the application.

1. Organize Assets
~~~~~~~~~~~~~~~~~~

Place all assets that should be exported into a PAK under a dedicated folder.

Example:

.. code-block:: text

    Content/
    └── MyAssets/
        ├── Meshes/
        ├── Materials/
        ├── Textures/
        └── Characters/

2. Create a Primary Asset Label
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the Content Browser, create a Primary Asset Label:

.. code-block:: text

    Right Click
     └── Miscellaneous
          └── Data Asset
               └── PrimaryAssetLabel

.. image:: ../images/pak-workflow/step-01.png
   :alt: Creating a Primary Asset Label from the Content Browser
   :align: center

Create a new asset:

.. image:: ../images/pak-workflow/step-02.png
   :alt: Creating the Primary Asset Label asset
   :align: center

Configure the asset:

.. image:: ../images/pak-workflow/step-03.png
   :alt: Configuring the Primary Asset Label
   :align: center

Choose a unique positive ``ChunkId`` greater than zero for each PAK. Chunk IDs
must be distinct; ID ``0`` is reserved.

Add all assets intended for packaging to the Data Asset's ``Explicit Asset``
and ``Explicit Blueprint`` fields. Conventional UAsset resources such as levels
and meshes go into ``Explicit Asset``. Character blueprints and similar assets
go into ``Explicit Blueprint``.

.. image:: ../images/pak-workflow/step-04.png
   :alt: Adding assets to Explicit Asset and Explicit Blueprint
   :align: center

3. Configure Packaging
~~~~~~~~~~~~~~~~~~~~~~

Open:

.. code-block:: text

    Edit
     └── Project Settings
          └── Packaging

.. image:: ../images/pak-workflow/step-05.png
   :alt: Opening Project Settings Packaging
   :align: center

In the Packaging panel, enable:

- ``Use Pak File``
- ``Generate Chunks``

Disable:

- ``Use Io Store``

If ``Use Io Store`` is enabled, disable it for this PAK workflow.

.. image:: ../images/pak-workflow/step-06.png
   :alt: Packaging settings for generating PAK chunks
   :align: center

4. Configure Asset Manager
~~~~~~~~~~~~~~~~~~~~~~~~~~

Open:

.. code-block:: text

    Edit
     └── Project Settings
          └── Asset Manager

In the Asset Manager panel, disable ``PrimaryAssetLabel - Is Editor Only``.

.. image:: ../images/pak-workflow/step-07.png
   :alt: Asset Manager PrimaryAssetLabel setting
   :align: center

5. Package the Project
~~~~~~~~~~~~~~~~~~~~~~

Package the project using the normal workflow:

.. code-block:: text

    Platforms
     └── Windows
          └── Package Project

.. image:: ../images/pak-workflow/step-08.png
   :alt: Packaging the Windows project
   :align: center

Choose an output directory:

.. image:: ../images/pak-workflow/step-09.png
   :alt: Choosing the package output directory
   :align: center

Wait for packaging to complete. Unreal Editor will report completion:

.. image:: ../images/pak-workflow/step-10.png
   :alt: Unreal Editor packaging completion notification
   :align: center

6. Locate the Generated PAK
~~~~~~~~~~~~~~~~~~~~~~~~~~~

After packaging, locate the generated PAK under:

.. code-block:: text

    OutputPath/
    └── Windows/
        └── YourProject/
            └── Content/
                └── Paks/

Example output:

.. image:: ../images/pak-workflow/step-11.png
   :alt: Generated PAK file in the packaged output
   :align: center

Step 2. Import into a Packaged Binary
-------------------------------------

Copy the generated ``.pak`` file into the packaged binary's PAK directory:

.. code-block:: text

    PackagedBinary/
    └── Windows/
        └── YourApplication/
            └── Content/
                └── Paks/
                    └── pakchunk111-Windows.pak

Multiple PAK files can be placed in this directory simultaneously.

.. image:: ../images/pak-workflow/step-12.png
   :alt: Copying a generated PAK into the packaged binary
   :align: center

Start the packaged binary normally. If its runtime implements automatic PAK
discovery, it will search ``Content/Paks/``, mount discovered PAK files, and
scan their assets during startup.

UnrealZoo Optional API
----------------------

.. note::

   The following TCP commands and C++ helpers are optional UnrealZoo
   extensions. They are not required by the standard Unreal PAK workflow and
   are not included in the open-source UnrealCV build.

Mount a Pak
~~~~~~~~~~~

.. code-block:: console

    vset /pak/mount D:/Paks/Assets_Win64.pak 0
    Mounted: D:/Paks/Assets_Win64.pak (Order: 0)

Optional C++ helper:

.. code-block:: cpp

    const bool bMounted = UPakMountBPLib::MountPakFile(
        TEXT("D:/Paks/Assets_Win64.pak"), 0);

Inspect, Scan, and Load
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: console

    vget /pak/mounted
    vget /pak/files 1
    vget /pak/assets_in_pak 1
    vset /pak/scan /Game/Assets 1
    vget /pak/assets /Game/Assets
    vget /pak/load /Game/Assets/BP_Chair.BP_Chair

Optional C++ helpers include ``ScanMountedAssets``, ``GetAllAssetsInPath``, and
``LoadAssetFromPak``.

Unmount a Pak
~~~~~~~~~~~~~

.. code-block:: console

    vset /pak/unmount 1

See Also
--------

- :doc:`../unrealcv_plus/reference/pak-commands` for the UnrealZoo optional command reference.
- `SimWorld custom PAK workflow <https://simworld.readthedocs.io/en/latest/customization/make_your_own_pak.html#>`_.

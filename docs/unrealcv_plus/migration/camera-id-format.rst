Camera ID Format Migration Guide
================================

Overview
--------

UnrealCV supports two camera identification formats: the legacy **Integer Format** and
the new **CID (Camera ID) Format**. This guide explains both formats and provides
migration strategies for existing codebases.

**Why Migrate?**

- Integer format is unstable across sessions (order-dependent)
- CID format is stable and tied to sensor instances
- CID format supports better debugging and logging
- Future features will require CID format

Camera ID Formats
-----------------

Integer Format (Legacy)
~~~~~~~~~~~~~~~~~~~~~~~

**Format:** ``0``, ``1``, ``2``, ...

**Description:**
Cameras are identified by their creation order index. The first camera created
is ID 0, second is ID 1, etc.

**Example:**

.. code-block:: cpp

   vget /camera/0/location
   vget /camera/1/view

**Issues:**
- Unstable: Camera order may change between sessions
- Unclear: ID doesn't indicate which camera
- Error-prone: Easy to reference wrong camera

CID Format (Recommended)
~~~~~~~~~~~~~~~~~~~~~~~~

**Format:** ``CID-ActorName-UUID``

**Components:**
- ``CID`` - Prefix indicating CID format
- ``ActorName`` - Name of the actor owning the sensor
- ``UUID`` - Unique identifier (2-digit hex, or random if collision)

**Examples:**

.. code-block::

   CID-FusionCamPawn-00
   CID-FusionCamPawn-01
   CID-Drone_BP-00
   CID-MyCameraActor-AB

**Generation:**

.. code-block:: cpp

   FString FCameraIDManager::GenerateUUID(UFusionCamSensor* Sensor)
   {
      FString ParentActorName = TEXT("Unknown");
      if (IsValid(Sensor))
      {
         AActor* Owner = Sensor->GetOwner();
         if (IsValid(Owner))
         {
            ParentActorName = Owner->GetName();
         }
      }

      FString GeneratedID;
      // Retry with incremental ID first
      for (int32 RetryCount = 0; RetryCount < 50; RetryCount++)
      {
         GeneratedID = FString::Printf(TEXT("CID-%s-%02x"),
            *ParentActorName, RetryCount);
         if (!UsedCameraIDs.Contains(GeneratedID))
         {
            break;
         }
      }
      // Fall back to random ID if collision
      if (UsedCameraIDs.Contains(GeneratedID))
      {
         uint32 RandomValue = FMath::Rand() & 0xff;
         GeneratedID = FString::Printf(TEXT("CID-%s-%02x"),
            *ParentActorName, RandomValue);
      }
      return GeneratedID;
   }

Backward Compatibility
----------------------

All UnrealCV functions accept both formats:

.. code-block:: cpp

   UFusionCamSensor* FCameraIDManager::GetSensorByAnyID(const FString& IDString)
   {
      // Detect format automatically
      if (IDString.StartsWith(TEXT("CID")))
      {
         // CID format
         return GetSensorByCID(IDString);
      }
      else
      {
         // Integer format
         int32 Index = FCString::Atoi(*IDString);
         return GetSensorByIndex(Index);
      }
   }

API Reference
-------------

FCameraIDManager
~~~~~~~~~~~~~~~~

**Header:** ``Source/UnrealCV/Private/BPFunctionLib/SensorBPLib.cpp``

GetSensorByAnyID
^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static UFusionCamSensor* GetSensorByAnyID(const FString& IDString);

Gets a sensor by either integer or CID format.

**Parameters:**

   - ``IDString`` (FString): Camera ID (integer or CID)

**Returns:** Pointer to sensor, or nullptr if not found

**Example:**

.. code-block:: cpp

   UFusionCamSensor* Sensor = FCameraIDManager::Get().GetSensorByAnyID("CID-FusionCamPawn-00");
   UFusionCamSensor* Sensor = FCameraIDManager::Get().GetSensorByAnyID("0");

GetIndexByAnyID
^^^^^^^^^^^^^^^

.. code-block:: cpp

   static int32 GetIndexByAnyID(const FString& IDString);

Gets the integer index for a camera ID.

**Parameters:**

   - ``IDString`` (FString): Camera ID (integer or CID)

**Returns:** Integer index, or -1 if invalid

GetNewFormatID
^^^^^^^^^^^^^^

.. code-block:: cpp

   static FString GetNewFormatID(UFusionCamSensor* Sensor);

Converts a sensor to CID format.

**Parameters:**

   - ``Sensor`` (UFusionCamSensor*): Sensor to convert

**Returns:** CID format string

GenerateUUID
^^^^^^^^^^^^

.. code-block:: cpp

   static FString GenerateUUID(UFusionCamSensor* Sensor);

Generates a unique CID for a sensor.

**Parameters:**

   - ``Sensor`` (UFusionCamSensor*): Sensor to generate ID for

**Returns:** Unique CID string

PrintIDMappings
^^^^^^^^^^^^^^^

.. code-block:: cpp

   static void PrintIDMappings() const;

Logs all camera ID mappings for debugging.

**Output Example:**

.. code-block::

   === Camera ID Mappings ===
     CID-FusionCamPawn-00
     CID-FusionCamPawn-01
     CID-Drone_BP-00

USensorBPLib Helpers
~~~~~~~~~~~~~~~~~~~~

**Header:** ``Source/UnrealCV/Public/BPFunctionLib/SensorBPLib.h``

GetFusionSensorList
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static TArray<UFusionCamSensor*> GetFusionSensorList();

Gets all sensors in the current world.

**Returns:** Array of all sensors

GetSensorById
^^^^^^^^^^^^^

.. code-block:: cpp

   static UFusionCamSensor* GetSensorById(int SensorId);

Gets sensor by integer ID.

**Parameters:**

   - ``SensorId`` (int): Integer camera ID

**Returns:** Sensor pointer, or nullptr

GetSensorByAnyID
^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static UFusionCamSensor* GetSensorByAnyID(const FString& IDString);

Gets sensor by CID or integer format.

GetIndexByAnyID
^^^^^^^^^^^^^^^

.. code-block:: cpp

   static int32 GetIndexByAnyID(const FString& IDString);

Gets index for any camera ID format.

GetFusionSensorListWithNewIDs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static TArray<FString> GetFusionSensorListWithNewIDs();

Gets all sensors with their CID format.

**Returns:** Array of all CIDs

GetSensorNewFormatID
^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static FString GetSensorNewFormatID(UFusionCamSensor* Sensor);

Gets CID for a specific sensor.

PrintCameraIDMappings
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static void PrintCameraIDMappings();

Prints all ID mappings to log.

Migration Strategies
--------------------

Strategy 1: Gradual Migration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Use ``GetSensorByAnyID()`` in new code
2. Keep integer IDs in existing code
3. Let the system handle format detection

**New Code:**

.. code-block:: cpp

   // Works with both formats
   UFusionCamSensor* Sensor = USensorBPLib::GetSensorByAnyID("CID-FusionCamPawn-00");

**Existing Code:**

.. code-block:: cpp

   // Still works
   UFusionCamSensor* Sensor = USensorBPLib::GetSensorById(0);

Strategy 2: Full Migration
~~~~~~~~~~~~~~~~~~~~~~~~~~

1. Convert all camera references to CID format
2. Use ``GetNewFormatID()`` to discover CIDs
3. Store CIDs in configuration files

**Discovery:**

.. code-block:: cpp

   // Print all CIDs for configuration
   USensorBPLib::PrintCameraIDMappings();

   // Get all CIDs programmatically
   TArray<FString> AllCIDs = USensorBPLib::GetFusionSensorListWithNewIDs();

**Configuration:**

.. code-block:: json

   {
      "cameras": [
         "CID-FusionCamPawn-00",
         "CID-FusionCamPawn-01"
      ]
   }

Strategy 3: UUID-Based Selection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For scripts and automation, use CID format:

.. code-block:: python

   from unrealcv import client

   # Get camera by CID
   res = client.request('vget /camera/CID-FusionCamPawn-00/location')

   # List all CIDs
   cameras = client.request('vget /camera/list')

Common Migration Patterns
-------------------------

**Pattern: Iterate All Cameras**

**Before (Integer):**

.. code-block:: cpp

   for (int32 i = 0; i < CameraCount; i++)
   {
      auto Sensor = USensorBPLib::GetSensorById(i);
      // ...
   }

**After (CID):**

.. code-block:: cpp

   TArray<UFusionCamSensor*> Sensors = USensorBPLib::GetFusionSensorList();
   for (auto Sensor : Sensors)
   {
      FString CID = USensorBPLib::GetSensorNewFormatID(Sensor);
      // ...
   }

**Pattern: Config-Driven Camera Selection**

**Before:**

.. code-block:: json

   {
      "primary_camera": 0,
      "secondary_camera": 1
   }

**After:**

.. code-block:: json

   {
      "primary_camera": "CID-FusionCamPawn-00",
      "secondary_camera": "CID-FusionCamPawn-01"
   }

**Pattern: Dynamic Camera Creation**

.. code-block:: cpp

   // Create camera via RecordingBPLib
   int32 CameraID = URecordingBPLib::CreateFreeCamera(WorldContext);

   // Get CID for new camera
   FString CID = USensorBPLib::GetSensorNewFormatID(
      USensorBPLib::GetSensorById(CameraID));

   UE_LOG(LogTemp, Log, TEXT("Created camera: %s"), *CID);

Troubleshooting
---------------

**Problem: Camera Not Found**

**Solution:** Use ``PrintCameraIDMappings()`` to verify IDs

.. code-block:: cpp

   USensorBPLib::PrintCameraIDMappings();

**Problem: Multiple Sensors with Same CID**

**Cause:** Sensor regeneration without world reset

**Solution:** Call ``Sync()`` to refresh mappings

.. code-block:: cpp

   FCameraIDManager::Get().Sync();

**Problem: Integer ID Out of Range**

**Cause:** Camera count changed between sessions

**Solution:** Use CID format or validate index

.. code-block:: cpp

   int32 Index = USensorBPLib::GetIndexByAnyID("0");
   if (Index >= 0)
   {
      // Valid
   }
   else
   {
      // Invalid
   }

Best Practices
--------------

1. **Use CID for new development**
   - Stable across sessions
   - Better debugging

2. **Store CIDs in configs**
   - Not integer indices
   - Survives engine restarts

3. **Use GetSensorByAnyID()**
   - Works with both formats
   - Future-proof code

4. **Print mappings during init**
   - For debugging
   - In development builds

5. **Handle null returns**
   - Always check for nullptr
   - Provide fallback logic

See Also
--------

- :doc:`../architecture/sensor-system` - Sensor architecture and APIs
- :doc:`../overview` - UnrealCV Dev For UnrealZoo camera feature summary

Annotation System Architecture
==============================

Overview
--------

The Annotation System provides instance segmentation coloring for distinguishing
multiple objects in camera captures. The system supports two annotation strategies:
Direct mode (CustomPrimitiveData) and Proxy mode (AnnotationComponent), each with
different performance characteristics and use cases.

**Header:** ``Source/UnrealCV/Public/Controller/ObjectAnnotator.h``

Architecture Diagram
--------------------

.. code::

   +---------------------+
   | FObjectAnnotator     |  <-- Static Facade
   +---------------------+
   | + AnnotateWorld()    |
   | + DeannotateWorld()  |
   | + SetAnnotationMode()|
   +--------+------------+
            |
            v
   +---------------------+       +---------------------+
   | FDirectAnnotator     |       | FProxyAnnotator      |
   +---------------------+       +---------------------+
   | CustomPrimitiveData  |       | AnnotationComponent  |
   | (Fast, Per-actor)   |       | (Batch, Component)   |
   +--------+------------+       +--------+------------+
            |                             |
            v                             v
   +---------------------+       +---------------------+
   | FColorGenerator     |       | FColorGenerator     |
   +---------------------+       +---------------------+
            |                             |
            v                             v
   +---------------------+       +---------------------+
   | StencilBPLib        |       | AnnotationComponent|
   | (CustomDepth)       |       | (Scene Proxy)      |
   +---------------------+       +---------------------+

FObjectAnnotator (Static Facade)
--------------------------------

``FObjectAnnotator`` is a static facade that delegates to either Direct or Proxy annotator
implementation based on the current annotation mode.

**Header:** ``Source/UnrealCV/Public/Controller/ObjectAnnotator.h``

Public API
~~~~~~~~~~

Initialize
^^^^^^^^^^

.. code-block:: cpp

   static void Initialize();

Initializes the annotation system and sets up the color generator.

**Note:** Called automatically on plugin startup.

Shutdown
^^^^^^^^

.. code-block:: cpp

   static void Shutdown();

Cleans up annotation resources.

AnnotateWorld
^^^^^^^^^^^^^

.. code-block:: cpp

   static void AnnotateWorld(UWorld* World);

Applies annotation colors to all actors in the world.

**Parameters:**

   - ``World`` (UWorld*): World to annotate

**Process:**

   1. Iterates all actors in the world
   2. Generates unique color for each actor
   3. Applies color using current annotation mode

DeannotateWorld
^^^^^^^^^^^^^^^

.. code-block:: cpp

   static void DeannotateWorld(UWorld* World);

Removes all annotation data from actors.

**Parameters:**

   - ``World`` (UWorld*): World to deannotate

SetAnnotationColor
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static int32 SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor);

Sets a specific annotation color for an actor.

**Parameters:**

   - ``Actor`` (AActor*): Actor to annotate
   - ``AnnotationColor`` (FColor): Color to assign

**Returns:** Number of primitives annotated

GetAnnotationColor
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static void GetAnnotationColor(AActor* Actor, FColor& AnnotationColor);

Retrieves the annotation color for an actor.

**Parameters:**

   - ``Actor`` (AActor*): Actor to query
   - ``AnnotationColor`` (FColor&): Output color

GetAnnotationColors
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static TMap<FString, FColor> GetAnnotationColors();

Gets all annotation color mappings.

**Returns:** Map of actor names to colors

SetAnnotationMode
^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static void SetAnnotationMode(bool bUseDirect);

Switches between Direct and Proxy annotation modes.

**Parameters:**

   - ``bUseDirect`` (bool): true for Direct mode, false for Proxy mode

**See Also:** :ref:`annotation-mode-comparison`

IsUsingDirectAnnotation
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   static bool IsUsingDirectAnnotation();

Checks current annotation mode.

**Returns:** true if using Direct mode

FDirectAnnotator
----------------

Direct mode uses ``CustomPrimitiveData`` to store annotation colors directly on
actor primitives. This approach is fast and has minimal overhead.

**Header:** ``Source/UnrealCV/Private/Controller/DirectAnnotator.h``

Implementation Details
~~~~~~~~~~~~~~~~~~~~~~

Direct annotator stores annotation data in ``CustomPrimitiveDataVector4`` slots:

.. code-block::

   Slot 4: (R, G, B, ActorID)
   - R: Red channel of annotation color
   - G: Green channel of annotation color
   - B: Blue channel of annotation color
   - ActorID: Unique actor identifier

Key Methods
^^^^^^^^^^^

AnnotateWorld
"""""""""""""

.. code-block:: cpp

   void AnnotateWorld(UWorld* World);

Iterates all actors and applies colors via CustomPrimitiveData.

**Process:**

   1. Get all actors from world
   2. Generate or retrieve color for each actor
   3. Set CustomPrimitiveDataVector4 on all primitive components
   4. Enable CustomDepth via StencilBPLib for stencil-based masking

SetAnnotationColor
""""""""""""""""""

.. code-block:: cpp

   int32 SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor);

Sets annotation data on all primitive components of an actor.

**Process:**

   1. Get all PrimitiveComponents from actor
   2. Set CustomPrimitiveDataVector4(4, AnnotationData)
   3. Store color in AnnotationColors map

GetAnnotationColor
""""""""""""""""""

.. code-block:: cpp

   void GetAnnotationColor(AActor* Actor, FColor& AnnotationColor);

Retrieves color from CustomPrimitiveData.

**Process:**

   1. Check AnnotationColors cache first
   2. If not cached, read from PrimitiveComponent CustomPrimitiveData

DeannotateWorld
"""""""""""""""

.. code-block:: cpp

   void DeannotateWorld(UWorld* World);

Clears all CustomPrimitiveData values.

**Process:**

   1. Iterate all actors
   2. Set CustomPrimitiveDataVector4(4, Zero) on all primitives
   3. Clear AnnotationColors map

FProxyAnnotator
---------------

Proxy mode uses ``AnnotationComponent`` to render annotation colors via a
material-based approach. This supports complex actors with multiple meshes.

**Header:** ``Source/UnrealCV/Private/Controller/ProxyAnnotator.h``

Implementation Details
~~~~~~~~~~~~~~~~~~~~~~

Proxy annotator attaches ``UAnnotationComponent`` to each mesh component.
The component renders a solid color using a custom material.

**Batch Processing:**

Proxy annotator uses batch processing with ``FlushRenderingCommands`` every
N actors to improve performance during world annotation.

Key Methods
^^^^^^^^^^^

AnnotateWorld
"""""""""""""

.. code-block:: cpp

   void AnnotateWorld(UWorld* World);

Attaches AnnotationComponents to all actors.

**Process:**

   1. Iterate all actors
   2. Create AnnotationComponent for each mesh
   3. Set annotation color via material parameter
   4. Batch process with FlushRenderingCommands (BatchSize=1)

SetAnnotationColor
""""""""""""""""""

.. code-block:: cpp

   int32 SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor);

Creates or updates AnnotationComponent on actor.

**Process:**

   1. Check for existing AnnotationComponent
   2. If none, create new AnnotationComponent
   3. If exists, update existing component
   4. Store color in AnnotationColors map

CreateAnnotationComponent
"""""""""""""""""""""""""

.. code-block:: cpp

   void CreateAnnotationComponent(AActor* Actor, FColor AnnotationColor);

Attaches new AnnotationComponent to mesh components.

**Process:**

   1. Get all MeshComponents from actor
   2. Create AnnotationComponent for each mesh
   3. Attach as child component
   4. Set annotation color via material parameter

**Skeletal Mesh Handling:**

   - Can be disabled via ``FUnrealcvServer::Config.DisableSKMAnnotation``
   - Supports SkeletalMeshComponent annotation when enabled

UpdateAnnotationComponent
"""""""""""""""""""""""""

.. code-block:: cpp

   void UpdateAnnotationComponent(AActor* Actor, FColor AnnotationColor);

Updates color on existing AnnotationComponent.

DeannotateWorld
"""""""""""""""

.. code-block:: cpp

   void DeannotateWorld(UWorld* World);

Destroys all AnnotationComponents.

**Process:**

   1. Find all AnnotationComponents
   2. Destroy each component
   3. Clear AnnotationColors map
   4. Flush rendering commands

UAnnotationComponent
--------------------

Component that renders annotation color via material.

**Header:** ``Source/UnrealCV/Public/Component/AnnotationComponent.h``

Public Methods
~~~~~~~~~~~~~~

SetAnnotationColor
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   void SetAnnotationColor(FColor AnnotationColor);

Sets the annotation color.

GetAnnotationColor
^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

   FColor GetAnnotationColor();

Gets the current annotation color.

ForceUpdate
^^^^^^^^^^^

.. code-block:: cpp

   void ForceUpdate();

Forces render state update.

Scene Proxy
^^^^^^^^^^^

The component creates scene proxies for different mesh types:

- ``CreateSceneProxy(UStaticMeshComponent*)``
- ``CreateSceneProxy(USkeletalMeshComponent*)``
- ``CreateSceneProxy(UGroomComponent*)``

FColorGenerator
---------------

Generates deterministic annotation colors from object indices.

**Header:** ``Source/UnrealCV/Public/Controller/ObjectAnnotator.h``

Algorithm
~~~~~~~~~

ColorGenerator uses bit manipulation to create channel-wise distinct colors:

.. code-block:: cpp

   class FColorGenerator
   {
   public:
      FColor GetColorFromColorMap(int32 ObjectIndex);

   private:
      int32 GetChannelValue(uint32 Index);
      void GetColors(int32 MaxVal, bool Fix1, bool Fix2, bool Fix3,
                     TArray<FColor>& ColorMap);
   };

**Color Generation Strategy:**

   - Each color channel varies at different bit positions
   - Ensures colors are visually distinct across indices
   - Deterministic: same index always produces same color

**Example Colors:**

   Index 0:  R=1,   G=2,   B=4   (1, 2, 4  = 0x010204)
   Index 1:  R=1,   G=4,   B=8   (1, 4, 8  = 0x010408)
   Index 2:  R=1,   G=8,   B=16  (1, 8, 16 = 0x010810)
   ...

Annotation Mode Comparison
--------------------------

.. _annotation-mode-comparison:

+---------------------------+--------------------+--------------------+
| Feature                   | Direct Mode        | Proxy Mode         |
+===========================+====================+====================+
| Storage                   | CustomPrimitiveData| AnnotationComponent|
+---------------------------+--------------------+--------------------+
| Performance               | Faster             | Slower             |
+---------------------------+--------------------+--------------------+
| Memory Overhead           | Low                | High               |
+---------------------------+--------------------+--------------------+
| Multi-mesh Actors         | One-shot set       | Per-mesh component |
+---------------------------+--------------------+--------------------+
| Complex Skeletal Meshes   | Supported          | Configurable       |
+---------------------------+--------------------+--------------------+
| Groom Support             | No                 | Yes                |
+---------------------------+--------------------+--------------------+
| Dynamic Updates           | Immediate          | Requires update    |
+---------------------------+--------------------+--------------------+

When to Use Each Mode
~~~~~~~~~~~~~~~~~~~~~

**Direct Mode (Recommended):**
   - Large number of static objects
   - Performance-critical scenarios
   - Simple mesh hierarchy
   - Actors with few primitive components

**Proxy Mode (Recommended):**
   - Complex skeletal meshes
   - Groom (hair/fur) annotation
   - Dynamic actors requiring frequent updates
   - When material-based rendering is needed

Blueprint Integration
---------------------

Use ``UAnnotationBPLib`` for Blueprint annotation control:

.. code-block:: cpp

   // Get annotation colors
   TMap<FString, FColor> Colors = FObjectAnnotator::GetAnnotationColors();

   // Check current mode
   bool bDirect = FObjectAnnotator::IsUsingDirectAnnotation();

   // Switch mode
   FObjectAnnotator::SetAnnotationMode(true);  // Direct
   FObjectAnnotator::SetAnnotationMode(false); // Proxy

See Also
--------

- :doc:`sensor-system` - Sensor system (reads annotation colors)
- :doc:`../reference/sensor-data-formats` - Annotation sensor output formats

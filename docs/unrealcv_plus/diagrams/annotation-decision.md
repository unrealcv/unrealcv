Annotation System Decision Tree
=============================

Decision guide for selecting Direct vs Proxy annotation mode.

Mode Overview
------------

+--------------------+----------------+----------------+----------------+
| Aspect             | Direct         | Proxy          |
+--------------------+----------------+----------------+----------------+
| Draw Calls         | O(N)           | O(1)           |
| Quality            | High           | Medium         |
| Memory             | O(N) materials | Constant       |
| Setup Complexity   | Low            | Medium         |
| Best For           | <50 objects    | >50 objects    |
+--------------------+----------------+----------------+----------------+

Decision Flowchart
-----------------

::

   Start: Need Annotation?
   ───────────────────────
          │
          ▼
   ┌───────────────────────┐
   │ How many annotated    │
   │ objects?              │
   └───────────────────────┘
          │
          ├─▶ < 10 objects ──────▶ USE DIRECT ✓
          │
          ├─▶ 10-50 objects ─────▶ Consider quality vs speed
          │                              │
          │                              ├─▶ High quality needed? ──▶ USE DIRECT ✓
          │                              │
          │                              └─▶ Speed priority ─────▶ USE PROXY ✓
          │
          └─▶ > 50 objects ──────▶ USE PROXY ✓

Detailed Decision Matrix
----------------------

+---------------------------+----------------+----------------+
| Scenario                  | Recommended    | Reason         |
+---------------------------+----------------+----------------|
| Single object mask         | Direct         | Simple, high   |
| 10 objects, static         | Direct         | Low overhead   |
| 20 objects, static         | Direct         | Acceptable     |
| 50 objects, static         | Either         | Depends on FPS |
| 100+ objects, any          | Proxy          | Required for   |
|                           |                | performance    |
| Real-time annotation       | Proxy          | Required for   |
|                           |                | frame rate     |
| Dynamic object changes     | Proxy          | Single update  |
| Complex materials          | Direct         | Better support|
| Many materials per object  | Direct         | Per-material   |
|                           |                | control        |
+---------------------------+----------------+----------------+

Direct Annotator Flow
--------------------

::

   ┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
   │ For each actor  │ ───▶ │ Create unique   │ ───▶ │ Render with     │
   │ to annotate     │      │ material        │      │ custom depth    │      │
   └─────────────────┘      └─────────────────┘      └─────────────────┘
                                                             │
                                                             ▼
                                                   ┌─────────────────┐
                                                   │ Extract color   │
                                                   │ from ID         │
                                                   └─────────────────┘

   Performance: O(N) where N = annotated actors
   - Material creation: N
   - Render passes: N
   - Memory: O(N) materials

Proxy Annotator Flow
-------------------

::

   ┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
   │ Enable custom   │ ───▶ │ Enable post-    │ ───▶ │ Post-process   │
   │ depth stencil   │      │ process pass    │      │ extracts IDs    │
   └─────────────────┘      └─────────────────┘      └─────────────────┘
                                                             │
                                                             ▼
                                                   ┌─────────────────┐
                                                   │ Color encoded   │
                                                   │ in RGB channels │
                                                   └─────────────────┘

   Performance: O(1) constant
   - Setup: 1 pass
   - Render: 1 pass
   - Memory: 1 post-process

Color Generation
---------------

Both modes use ColorGenerator for deterministic colors:

::

   Object ID ──▶ ColorGenerator ──▶ Unique Color
      │              │                  │
      │              ▼                  ▼
      │         Channel-wise        R: high bits
      │         bit patterns        G: mid bits
      │                             B: low bits

   Formula: Color(ID) = (ID >> 16, (ID >> 8) & 255, ID & 255)

Mode Switching
-------------

::

   ┌─────────────────┐      ┌─────────────────┐
   │ FObjectAnnotator│ ───▶ │ SetAnnotationMode│
   │ Static Facade   │      │ (EAnnotationMode)│
   └─────────────────┘      └─────────────────┘
                                     │
                    ┌────────────────┼────────────────┐
                    ▼                ▼                ▼
           ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
           │ Direct       │  │ Proxy        │  │ Auto         │
           │ Annotator    │  │ Annotator    │  │ (based on N) │
           └──────────────┘  └──────────────┘  └──────────────┘

Example Usage
------------

.. code-block:: cpp

   // Switch to Direct for quality
   FObjectAnnotator::SetAnnotationMode(EAnnotationMode::Direct);

   // Annotate objects
   for (AActor* Actor : TargetActors)
   {
       FObjectAnnotator::AnnotateActor(Actor);
   }

   // Switch to Proxy for many objects
   FObjectAnnotator::SetAnnotationMode(EAnnotationMode::Proxy);

   // Annotate scene
   FObjectAnnotator::AnnotateAllActors();

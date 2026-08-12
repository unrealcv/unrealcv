Annotation System Decision Tree
===============================

Use this page as a compact guide for selecting the annotation implementation
that fits a scene. For implementation details, see
:doc:`../architecture/annotation-system`.

Mode Comparison
---------------

.. list-table::
   :header-rows: 1
   :widths: 30 35 35

   * - Aspect
     - Direct
     - Proxy
   * - Setup
     - Per-object annotation state.
     - Shared proxy or post-process path.
   * - Best fit
     - Small scenes and maximum material fidelity.
     - Large scenes and repeated capture.
   * - Update cost
     - Grows with annotated objects.
     - More nearly constant after setup.
   * - Tradeoff
     - Higher per-object overhead.
     - Additional proxy-path constraints.

Decision Flow
-------------

.. code-block:: text

    Need annotation?
          |
          v
    How many objects change or render in the annotation pass?
          |
          +-- Small, quality-sensitive scene ------> Direct
          |
          +-- Large or frequently captured scene -> Proxy
          |
          +-- Uncertain --------------------------> Benchmark both

Direct Flow
-----------

.. code-block:: text

    Actor list
        |
        v
    Apply per-object annotation state
        |
        v
    Render annotation output

Proxy Flow
----------

.. code-block:: text

    Actor IDs / stencil state
        |
        v
    Shared proxy or post-process pass
        |
        v
    Decode IDs into annotation colors

Selection Checklist
-------------------

- Measure the actual scene instead of choosing only from object count.
- Include skeletal meshes, translucent materials, and Groom components in QA.
- Verify that annotation IDs remain stable after spawning or destroying actors.
- Compare output correctness before comparing frame time.

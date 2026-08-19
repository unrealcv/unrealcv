Scene perception
=================

UnrealCV Dev for UnrealZoo provides structured scene state for agents. It is
separate from the open-source command reference.

Occupancy
---------

All occupancy arrays are boolean NumPy data with axis order ``x,y_up,z``.
Coordinates are meters; ``origin_cm`` and ``yaw_degrees`` place the grid in UE.

Profiles
~~~~~~~~

``lingo_vis`` is ``8 x 2 x 12 m`` at 10 cm (shape ``400 x 100 x 600``).

.. image:: ../images/scene-occupancy-lingo-vis.png
   :alt: lingo_vis occupancy visualization
   :width: 100%

``lingo_train`` is ``6 x 2 x 8 m`` at 10 cm (shape ``300 x 100 x 400``).

.. image:: ../images/scene-occupancy-lingo-train.png
   :alt: lingo_train occupancy visualization
   :width: 100%

Commands:

.. code-block:: text

   vget /scene/occupancy npy lingo_vis mesh
   vget /scene/occupancy/spec lingo_vis mesh
   vget /scene/occupancy_shared lingo_vis mesh

Use ``bounds`` for fast AABB occupancy and ``mesh`` for triangle-based surface
occupancy.

Custom region
~~~~~~~~~~~~~

Use explicit bounds instead of a profile. Order is
``min_x max_x min_y max_y min_z max_z voxel_size_m``.

.. code-block:: text

   vget /scene/occupancy_region npy mesh -150 150 -150 150 -150 150 1
   vget /scene/occupancy_region/spec mesh -150 150 -150 150 -150 150 1
   vget /scene/occupancy_shared_region mesh -150 150 -150 150 -150 150 1

This example covers 300 m in every axis at 1 m resolution.

.. image:: ../images/scene-occupancy-region-300m.png
   :alt: Custom 300 meter occupancy region visualization
   :width: 100%

Python client:

.. code-block:: python

   grid = client.get_scene_occupancy_region(
       (-150, -150, -150), (150, 150, 150),
       voxel_size_m=1.0, method="mesh")
   spec = client.get_scene_occupancy_region_spec(
       (-150, -150, -150), (150, 150, 150), voxel_size_m=1.0)

The shape is ``ceil((max_m - min_m) / voxel_size_m)``. Choose voxel size to
keep the grid within the runtime voxel budget.

Native perception
------------------

``vget /scene/perception`` returns compact JSON for agent use:

.. code-block:: text

   vget /scene/perception
   vget /scene/perception 1000 16 8

The response includes agent pose, nearby object transforms and bounds, tags,
materials, line of sight, blockers, navigation reachability, radial rays, and
occupancy capability metadata. It does not embed the voxel grid.

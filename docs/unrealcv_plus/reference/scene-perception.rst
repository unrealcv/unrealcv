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

The profiles support the LINGO occupancy-grid convention used by the
LINGO project: boolean grids describe scene occupancy in a fixed local frame,
with a choice between fast component-bounds rasterization and mesh-surface
rasterization. See the
`LINGO release repository <https://github.com/mileret/lingo-release>`_.
The profile names are compatibility presets, not
different data types.

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

NPY output
~~~~~~~~~~

``vget /scene/occupancy npy lingo_vis mesh`` returns a NumPy ``bool`` array.
For example:

.. code-block:: python

   grid = np.load("lingo_vis.npy", allow_pickle=False)
   print(grid.shape, grid.dtype)
   # (400, 100, 600) bool

The dimensions are ``(x, y_up, z)``:

* axis 0: 400 voxels across ``-4 .. 4 m`` (10 cm per voxel);
* axis 1: 100 voxels across ``0 .. 2 m`` height;
* axis 2: 600 voxels across ``-6 .. 6 m``.

The value ``grid[x, y_up, z]`` is ``True`` when that voxel is occupied. The
``lingo_train`` profile is ``(300, 100, 400)`` with bounds ``6 x 2 x 8 m``.
The exact bounds and shape can be queried with ``/scene/occupancy/spec``.

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

Real output
~~~~~~~~~~~

Captured from the Tokyo map with ``vget /scene/perception 1000 4 4``:

.. code-block:: json

   {
     "schema_version": "unrealcv.native_perception.v1",
     "world_name": "Tokyo",
     "search_radius_cm": 1000,
     "max_objects": 4,
     "agent": {
       "position_cm": {"x": -5160.63, "y": -1029.99, "z": 138.64},
       "rotation": {"pitch": 0.0, "yaw": -1.002, "roll": 0.0}
     },
     "agent_nav": {
       "available": true,
       "reachable": false,
       "reason": "target_not_on_navmesh"
     },
     "objects_found": 138,
     "objects_returned": 4,
     "objects_truncated": true,
     "objects": [{
       "name": "StaticMeshActor_3002",
       "distance_cm": 114.72,
       "line_of_sight": true,
       "relative_to_agent_cm": {"x": -102.61, "y": 4.63, "z": 51.11}
     }],
     "rays": [{
       "yaw_degrees": 90.0,
       "hit": true,
       "actor": "StaticMeshActor_1349",
       "distance_cm": 193.86
     }],
     "ray_channel": "Visibility",
     "units": "world vectors and distances are centimeters; angles are degrees"
   }

Here ``objects_found`` is the number of actors inside the search radius;
``objects_returned`` is capped by ``max_objects``. ``line_of_sight`` is the
visibility result from the agent camera. A ``blocked_by`` field is added when
another actor occludes the target. Rays provide radial visibility checks, while
``agent_nav`` reports navigation projection and reachability. This structured
snapshot is intended to give an agent useful scene state without image
recognition.

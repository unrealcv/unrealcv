UnrealCV Dev for UnrealZoo scene perception
============================================

This page documents the closed-build scene perception surface in UnrealCV Dev
For UnrealZoo. It is separate from the open-source server command reference:
see :doc:`../../reference/commands` for the public command contract.

The design goal is to let an agent query the scene through structured world
state instead of inferring everything from rendered images. The runtime can
return compact object, navigation, line-of-sight, blocker, ray, and occupancy
metadata without embedding a large voxel array in the agent context.

Occupancy grids
---------------

Occupancy commands produce a boolean NumPy array with axis order ``x,y_up,z``.
The two built-in LINGO profiles are local grids, not a 300 m world map:

* ``lingo_vis``: bounds ``[-4, 0, -6]`` to ``[4, 2, 6]`` meters, shape
  ``(400, 100, 600)`` (10 cm voxels).
* ``lingo_train``: bounds ``[-3, 0, -4]`` to ``[3, 2, 4]`` meters, shape
  ``(300, 100, 400)`` (10 cm voxels).

The visualization below is a mesh-surface rendering of the ``lingo_vis``
style local grid. Colors encode voxel height; they are for inspection only and
are not part of the binary command response.

.. image:: ../images/scene-occupancy-3d-views.png
   :alt: Four views of a height-colored 10 cm occupancy surface
   :align: center
   :width: 100%

The current command contract does not expose a 300 m profile. A 300 m view
would require a new profile with an explicit origin, shape, voxel size, and a
voxel-count budget. Do not treat the local visualization above as 300 m of
coverage.

``vget /scene/occupancy``
~~~~~~~~~~~~~~~~~~~~~~~~~

Return the grid as an NPY binary payload, or save it to a server-side file.
The short forms use the default ``bounds`` method:

.. code-block:: text

   vget /scene/occupancy npy lingo_vis
   vget /scene/occupancy C:/captures/scene_occ.npy lingo_vis
   vget /scene/occupancy npy lingo_vis mesh

The complete form is:

.. code-block:: text

   vget /scene/occupancy [npy|filename] [profile] [method]
       [origin_cm_x] [origin_cm_y] [origin_cm_z] [yaw_degrees]
       [include_dynamic]

``method`` is ``bounds`` or ``mesh``. ``bounds`` fills voxels intersecting
component bounds and is a fast coarse representation. ``mesh`` rasterizes
render triangles, with physics-mesh fallback where render triangles are not
available. ``include_dynamic`` is ``0`` or ``1``.

Use ``origin_cm`` and ``yaw_degrees`` to place and rotate the grid around an
agent or another world-space anchor. Unreal world positions and origins are in
centimeters; the profile extents and voxel coordinates are in meters.

``vget /scene/occupancy/spec``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Query the profile metadata before allocating a consumer buffer:

.. code-block:: text

   vget /scene/occupancy/spec lingo_vis
   vget /scene/occupancy/spec lingo_vis mesh

The response describes ``profile``, ``method``, ``shape``, ``min_m``,
``max_m``, ``voxel_size_m``, ``origin_cm``, ``yaw_degrees``, and the dynamic
component policy.

``vget /scene/occupancy_shared``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use Windows named shared memory instead of returning the NPY bytes over TCP:

.. code-block:: text

   vget /scene/occupancy_shared lingo_vis
   vget /scene/occupancy_shared lingo_vis mesh
   vget /scene/occupancy_shared lingo_vis mesh 0 0 0 0 0

The JSON descriptor contains ``transport``, ``name``, ``num_bytes``,
``version``, ``offset_bytes``, ``shape``, ``dtype`` (``bool``), ``layout``
(``XYZ``), ``axis_order``, profile bounds, origin, yaw, and lifetime. The named
mapping remains valid until the next occupancy request overwrites that mapping
or the UnrealCV server exits.

Native agent perception
-----------------------

``vget /scene/perception`` returns a compact JSON snapshot centered on the
main player camera. It is intended as an agent-facing alternative to parsing
images for basic spatial awareness.

.. code-block:: text

   vget /scene/perception
   vget /scene/perception 1000 16 8

Arguments are ``radius_cm``, ``max_objects``, and ``max_rays``. The runtime
clamps them to 100--100000 cm, 1--64 objects, and 0--16 radial visibility rays.
``max_rays=0`` disables the ray array.

The top-level response includes:

* ``schema_version``: ``unrealcv.native_perception.v1``.
* ``world_name``, ``world_path``, ``gravity_z_cm_s2`` and query limits.
* ``agent``: camera position, rotation, and forward/right/up vectors.
* ``agent_nav``: navigation projection and reachability information when a
  navigation system is available.
* ``objects``: nearest actors, including ``name``, class and object paths,
  transform, ``bbox``, world and agent-relative offsets, distance, azimuth,
  elevation, ``line_of_sight``, annotation color, materials, tags, and an
  optional ``blocked_by`` object.
* ``objects_found``, ``objects_returned`` and ``objects_truncated`` so an agent
  can detect whether the result was capped by ``max_objects``.
* ``rays`` and ``ray_channel``: radial visibility hit records using the
  ``Visibility`` channel. Each hit can include location, normal, distance,
  actor, component, physical material, and walkability.
* ``occupancy``: discoverable occupancy capability metadata and the commands
  above. The perception response intentionally does not embed the voxel grid.

All world vectors and distances in this response are centimeters; angles are
degrees. Use occupancy commands when a planning or geometric consumer needs a
voxel array, and use perception when an agent needs a bounded, low-latency
scene summary.

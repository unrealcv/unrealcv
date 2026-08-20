MQRC rendering comparison
=========================

Movie Quality Render Component (MQRC) renders a lit image through the
high-quality path, including temporal accumulation, Lumen updates, exposure,
and the final tone-map. For a fair visual comparison, capture BaseCam
``vget /camera/0/lit png`` and MQRC ``vget /camera/0/mqrc/lit png`` from the
same camera transform, resolution, map, and frame after identical warm-up.

.. image:: ../images/mqrc-vs-basecam-tokyo.png
   :alt: Same-position BaseCam lit and MQRC lit comparison showing tone mapping
   :width: 100%

The BaseCam path is useful for low-latency observation. MQRC is intended when
the image itself matters: temporal anti-aliasing and the complete tone-map
preserve highlight roll-off and shadow detail, avoiding the under-exposed look
that can occur when a high-dynamic-range result is read without the final
post-process.

The comparison above was captured from the packaged Tokyo map at camera
location ``(-5160.632, -1029.995, 138.643)`` and rotation
``(0, -1.002, 0)``. Both sides use camera 0 at 640 x 480 after ten warm-up
captures. The MQRC side was returned by ``scene.capture_view`` with
``render_source=mqrc`` and ``uses_base_camera_sensor=false``.

Recommended sequence::

    vset /camera/0/location X Y Z
    vset /camera/0/rotation P Y R
    vset /mqrc/render_immediately 1
    # discard warm-up frames before saving the comparison pair
    vget /camera/0/lit png
    vget /camera/0/mqrc/lit png

MQRC controls and transport benchmarks are listed in
:doc:`commands` and :doc:`../../reference/unrealzoo_capture_transport`.

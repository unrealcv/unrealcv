UnrealCV Dev For UnrealZoo Shared-Memory Transport
==================================================

The shared-memory implementation in UnrealCV Dev for
`UnrealZoo <https://github.com/UnrealZoo>`_ draws on the SPEAR project paper
(`https://arxiv.org/abs/2607.06701`) and source repository
(`https://github.com/spear-sim/spear`).

This section reports measurements from the August 27, 2026 packaged build.
UnrealCV listened on port 9001; port 9000 was not used.
The test environment was:

* Windows 11
* AMD Ryzen 9 5950X, 16 cores and 32 logical processors
* NVIDIA RTX 6000D
* Epic Games Unreal Engine 5.7.4
* 3 independent rounds per table row and transport
* 10 warm-up captures per transport at the start of every round
* 20 measured captures per round, or 60 measured captures per table cell

TCP and shared-memory requests were alternated to reduce request-order bias.
The reported latency is the arithmetic mean of all 60 end-to-end client
samples. TCP timing includes receipt of the complete 32-bit BGRA BMP payload.
Shared-memory timing includes the request, JSON parsing, opening the named
mapping, and copying every mapped byte. FPS is ``1000 / mean latency`` and
therefore represents serialized client acquisition throughput, not unloaded
viewport frame rate.

The standard-camera rows use ``vget /camera/0/lit bmp`` versus
``vget /camera/0/lit_shared``. At 4K, both transports carry the same
33,177,600 pixel bytes; the BMP response has an additional 54-byte header.

.. list-table:: Standard Camera
   :header-rows: 1
   :widths: 20 20 22 18

   * - Resolution
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 38.86
     - 66.28
     - 1.71x
   * - 720p (1280x720)
     - 26.57
     - 62.06
     - 2.34x
   * - 1080p (1920x1080)
     - 16.35
     - 62.28
     - 3.81x
   * - 2K (2560x1440)
     - 9.91
     - 48.94
     - 4.94x
   * - 4K (3840x2160)
     - 3.85
     - 21.78
     - 5.65x
   * - 8K (7680x4320)
     - 1.21
     - 6.76
     - 5.59x

MQRC
----

MQRC uses ``vget /camera/0/mqrc/lit bmp`` versus
``vget /camera/0/mqrc/lit_shared``. Both paths return the same BGRA8 pixel
payload, so the comparison includes MQRC rendering and HDR-to-BGRA8 conversion
as well as transport. All 720 measured MQRC requests below succeeded.

.. list-table:: Movie Quality Render Component
   :header-rows: 1
   :widths: 20 20 22 18

   * - Resolution
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 23.33
     - 39.94
     - 1.71x
   * - 720p (1280x720)
     - 16.21
     - 28.50
     - 1.76x
   * - 1080p (1920x1080)
     - 10.01
     - 17.78
     - 1.78x
   * - 2K (2560x1440)
     - 5.31
     - 10.91
     - 2.05x
   * - 4K (3840x2160)
     - 3.29
     - 6.61
     - 2.01x
   * - 8K (7680x4320)
     - 0.59
     - 1.60
     - 2.69x

Multi-camera capture
--------------------

The multi-camera test uses 640x480 and one ``vbatch N`` request containing
one capture command for each of N distinct cameras. FPS is the number of
complete synchronized N-camera capture rounds per second.

.. list-table:: Synchronized multi-camera FPS
   :header-rows: 1
   :widths: 15 20 24 18

   * - Cameras
     - Lit FPS
     - Lit shared FPS
     - Speedup
   * - 1
     - 20.29
     - 30.13
     - 1.48x
   * - 2
     - 16.30
     - 24.47
     - 1.50x
   * - 3
     - 13.09
     - 23.46
     - 1.79x
   * - 4
     - 10.30
     - 18.83
     - 1.83x
   * - 5
     - 8.34
     - 17.10
     - 2.05x
   * - 6
     - 7.59
     - 16.13
     - 2.12x
   * - 7
     - 5.94
     - 13.12
     - 2.21x
   * - 8
     - 5.88
     - 12.48
     - 2.12x
   * - 9
     - 5.18
     - 11.57
     - 2.23x
   * - 10
     - 4.76
     - 11.52
     - 2.42x

Cross-simulator comparison
--------------------------

The following table is an additional product-level comparison against
`CARLA <https://carla.org/>`_, `SimWorld <https://simworld.org/>`_, and
`AirSim <https://microsoft.github.io/AirSim/>`_. It does not replace the
UnrealZoo measurements above.
The UnrealCV shared-memory column uses the same packaged-build Standard Camera
measurements reported above and ``vget /camera/0/lit_shared``.

Each result uses three rounds, ten warm-up captures per round, and twenty
measured captures per round, for 60 measured samples per cell. FPS is
``1000 / arithmetic mean client latency``. Simulators ran serially and
offscreen.

.. list-table:: Effective FPS
   :header-rows: 1
   :widths: 16 24 20 20 20

   * - Resolution
     - UnrealCV shared
     - CARLA (UE4)
     - SimWorld
     - AirSim (UE4)
   * - 640x480
     - 66.28
     - 69.94
     - 18.24
     - 68.91
   * - 1280x720
     - 62.06
     - 49.09
     - 11.56
     - 37.85
   * - 1920x1080
     - 62.28
     - 27.07
     - 7.81
     - 12.99
   * - 2560x1440
     - 48.94
     - 17.41
     - 5.13
     - 8.03
   * - 3840x2160
     - 21.78
     - 8.38
     - 2.58
     - 3.89
   * - 7680x4320
     - 6.76
     - 2.13
     - 0.70
     - 1.04


AirSim returns three-byte RGB pixels; the other columns use four-byte BGRA
pixels. The simulators use different scenes and rendering pipelines, so this
is an end-to-end acquisition comparison rather than a transport-only test.

* CARLA: `paper <https://arxiv.org/abs/1711.03938>`__,
  `repository <https://github.com/carla-simulator/carla>`__
* SimWorld: `documentation <https://simworld.readthedocs.io/en/latest/>`__,
  `repository <https://github.com/SimWorld-AI/SimWorld>`__
* AirSim: `paper <https://arxiv.org/abs/1705.05065>`__,
  `repository <https://github.com/microsoft/AirSim>`__

UnrealCV Dev For UnrealZoo Shared-Memory Transport
==================================================

The shared-memory implementation in UnrealCV Dev for
`UnrealZoo <https://github.com/UnrealZoo>`_ draws on the SPEAR project paper
(`https://arxiv.org/abs/2607.06701`) and source repository
(`https://github.com/spear-sim/spear`).

This section reports measurements from a packaged UnrealZoo build. The test
environment was:

* Windows 11
* AMD Ryzen 9 5950X, 16 cores and 32 logical processors
* NVIDIA RTX 6000D
* Epic Games Unreal Engine 5.7.4
* A fixed scene and identical camera pose for every measurement
* 3 independent runs and 180 measured samples per table cell

TCP and shared-memory requests were alternated. FPS is based on mean client
latency across all 180 samples; ``+/-`` is the standard deviation across the
three independent runs. Both paths include receiving or copying the complete
image.

The standard-camera rows compare TCP and shared-memory lit capture.

.. list-table:: Standard Camera
   :header-rows: 1
   :widths: 20 20 22 18

   * - Resolution
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 40.65 +/- 1.13
     - 64.00 +/- 1.85
     - 1.57x
   * - 720p (1280x720)
     - 26.19 +/- 0.39
     - 69.70 +/- 1.30
     - 2.66x
   * - 1080p (1920x1080)
     - 14.95 +/- 0.18
     - 62.99 +/- 2.42
     - 4.21x
   * - 2K (2560x1440)
     - 9.20 +/- 0.39
     - 53.46 +/- 0.53
     - 5.81x
   * - 4K (3840x2160)
     - 4.69 +/- 0.31
     - 29.59 +/- 1.39
     - 6.31x
   * - 8K (7680x4320)
     - 1.25 +/- 0.11
     - 7.68 +/- 0.54
     - 6.14x

MQRC
----

MQRC uses ``vget /camera/0/mqrc/lit bmp`` versus
``vget /camera/0/mqrc/lit_shared``. Both paths return the same BGRA8 pixel
payload, so the comparison includes MQRC rendering and HDR-to-BGRA8 conversion
as well as transport. All 2,160 measured MQRC requests below succeeded.

.. list-table:: Movie Quality Render Component
   :header-rows: 1
   :widths: 20 20 22 18

   * - Resolution
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 25.45 +/- 0.74
     - 40.12 +/- 1.30
     - 1.58x
   * - 720p (1280x720)
     - 15.85 +/- 0.35
     - 30.22 +/- 0.63
     - 1.91x
   * - 1080p (1920x1080)
     - 9.71 +/- 0.35
     - 19.24 +/- 0.37
     - 1.98x
   * - 2K (2560x1440)
     - 6.27 +/- 0.03
     - 12.81 +/- 0.19
     - 2.04x
   * - 4K (3840x2160)
     - 3.28 +/- 0.17
     - 7.01 +/- 0.12
     - 2.14x
   * - 8K (7680x4320)
     - 0.77 +/- 0.05
     - 1.78 +/- 0.05
     - 2.30x

Multi-camera capture
--------------------

The multi-camera test uses 640x480 with the same scene and camera pose. Each
camera count was tested independently. N=1 reuses the Standard Camera 480p
result. FPS is the number of complete synchronized capture rounds per second.

.. list-table:: Synchronized multi-camera FPS
   :header-rows: 1
   :widths: 15 20 24 18

   * - Cameras
     - Lit FPS
     - Lit shared FPS
     - Speedup
   * - 1
     - 40.65 +/- 1.13
     - 64.00 +/- 1.85
     - 1.57x
   * - 2
     - 13.31 +/- 1.93
     - 21.13 +/- 3.81
     - 1.59x
   * - 3
     - 13.44 +/- 0.13
     - 24.04 +/- 0.42
     - 1.79x
   * - 4
     - 10.97 +/- 0.17
     - 20.68 +/- 0.09
     - 1.89x
   * - 5
     - 9.06 +/- 0.21
     - 17.46 +/- 0.46
     - 1.93x
   * - 6
     - 8.23 +/- 0.22
     - 16.92 +/- 0.74
     - 2.06x
   * - 7
     - 7.04 +/- 0.19
     - 14.96 +/- 0.57
     - 2.13x
   * - 8
     - 6.31 +/- 0.46
     - 13.49 +/- 0.19
     - 2.14x
   * - 9
     - 5.88 +/- 0.16
     - 12.41 +/- 0.30
     - 2.11x
   * - 10
     - 5.33 +/- 0.17
     - 11.50 +/- 0.19
     - 2.16x

Cross-simulator comparison
--------------------------

The following table is an additional product-level comparison against
`CARLA <https://carla.org/>`_, `SimWorld <https://simworld.org/>`_, and
`AirSim <https://microsoft.github.io/AirSim/>`_. It does not replace the
UnrealZoo measurements above.
The UnrealCV shared-memory column uses the same packaged-build Standard Camera
measurements reported above and ``vget /camera/0/lit_shared``.

The UnrealCV results use the controlled 180-sample procedure above. The other
simulators retain their original three-round, 60-sample measurements.
Simulators ran serially and offscreen.

.. list-table:: Effective FPS
   :header-rows: 1
   :widths: 16 24 20 20 20

   * - Resolution
     - UnrealCV shared
     - CARLA (UE4)
     - SimWorld
     - AirSim (UE4)
   * - 640x480
     - 64.00
     - 69.94
     - 18.24
     - 68.91
   * - 1280x720
     - 69.70
     - 49.09
     - 11.56
     - 37.85
   * - 1920x1080
     - 62.99
     - 27.07
     - 7.81
     - 12.99
   * - 2560x1440
     - 53.46
     - 17.41
     - 5.13
     - 8.03
   * - 3840x2160
     - 29.59
     - 8.38
     - 2.58
     - 3.89
   * - 7680x4320
     - 7.68
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

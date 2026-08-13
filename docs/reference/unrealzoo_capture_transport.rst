UnrealZoo shared-memory transport
=================================

The shared-memory implementation in UnrealCV Dev for
`UnrealZoo <https://github.com/UnrealZoo>`_ draws on the
`SPEAR project paper <https://arxiv.org/abs/2607.06701>`_ and
`source repository <https://github.com/spear-sim/spear>`_.

This section is a closed-build measurement of the UnrealZoo distribution.
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
``vget /camera/0/lit_shared``. Panorama rows use
``vget /camera/0/panoramic bmp WIDTH HEIGHT`` versus
``vget /camera/0/panoramic_shared WIDTH HEIGHT``. At 4K, both transports carry
the same 33,177,600 pixel bytes; the BMP response has an additional 54-byte
header.

.. list-table:: Standard Camera
   :header-rows: 1
   :widths: 14 16 16 14 14 14

   * - Resolution
     - TCP mean
     - Shared mean
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 24.62 ms
     - 15.61 ms
     - 40.62
     - 64.05
     - 1.58x
   * - 720p (1280x720)
     - 40.39 ms
     - 12.77 ms
     - 24.76
     - 78.30
     - 3.16x
   * - 1080p (1920x1080)
     - 62.63 ms
     - 11.14 ms
     - 15.97
     - 89.77
     - 5.62x
   * - 2K (2560x1440)
     - 93.75 ms
     - 10.39 ms
     - 10.67
     - 96.20
     - 9.02x
   * - 4K (3840x2160)
     - 224.76 ms
     - 15.34 ms
     - 4.45
     - 65.21
     - 14.66x
   * - 8K (7680x4320)
     - 713.61 ms
     - 55.25 ms
     - 1.40
     - 18.10
     - 12.92x

.. list-table:: Panorama
   :header-rows: 1
   :widths: 14 16 16 14 14 14

   * - Resolution
     - TCP mean
     - Shared mean
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 66.21 ms
     - 46.74 ms
     - 15.10
     - 21.39
     - 1.42x
   * - 720p (1280x720)
     - 77.90 ms
     - 49.40 ms
     - 12.84
     - 20.24
     - 1.58x
   * - 1080p (1920x1080)
     - 103.37 ms
     - 56.70 ms
     - 9.67
     - 17.64
     - 1.82x
   * - 2K (2560x1440)
     - 136.46 ms
     - 61.85 ms
     - 7.33
     - 16.17
     - 2.21x
   * - 4K (3840x2160)
     - 250.10 ms
     - 77.47 ms
     - 4.00
     - 12.91
     - 3.23x
   * - 8K (7680x4320)
     - 878.86 ms
     - 177.02 ms
     - 1.14
     - 5.65
     - 4.96x

MQRC
----

MQRC uses ``vget /camera/0/mqrc/lit bmp`` versus
``vget /camera/0/mqrc/lit_shared``. Both paths return the same BGRA8 pixel
payload, so the comparison includes MQRC rendering and HDR-to-BGRA8 conversion
as well as transport. All 720 measured MQRC requests below succeeded.

.. list-table:: Movie Quality Render Component
   :header-rows: 1
   :widths: 14 16 16 14 14 14

   * - Resolution
     - TCP mean
     - Shared mean
     - TCP FPS
     - Shared FPS
     - Speedup
   * - 480p (640x480)
     - 41.73 ms
     - 20.89 ms
     - 23.96
     - 47.87
     - 2.00x
   * - 720p (1280x720)
     - 59.10 ms
     - 26.80 ms
     - 16.92
     - 37.31
     - 2.21x
   * - 1080p (1920x1080)
     - 112.45 ms
     - 36.62 ms
     - 8.89
     - 27.30
     - 3.07x
   * - 2K (2560x1440)
     - 170.31 ms
     - 54.72 ms
     - 5.87
     - 18.27
     - 3.11x
   * - 4K (3840x2160)
     - 298.29 ms
     - 108.78 ms
     - 3.35
     - 9.19
     - 2.74x
   * - 8K (7680x4320)
     - 1243.12 ms
     - 350.26 ms
     - 0.80
     - 2.85
     - 3.55x


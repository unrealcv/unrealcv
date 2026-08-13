`UnrealZoo <https://github.com/UnrealZoo>`_ Capture Transport Benchmark
========================================================================

This section is a closed-build measurement of the UnrealZoo distribution. 

The following result was measured on August, 2026 with the shared-memory implementation merged by PR #334.
The test environment was:

* Windows 11
* AMD Ryzen 9 5950X, 16 cores and 32 logical processors
* NVIDIA RTX 6000D
* Epic Games Unreal Engine 5.7.4
* 2 warm-up captures followed by 5 measured captures per transport and modality

The standard-camera rows use ``vget /camera/0/lit bmp`` versus ``lit_shared``. Panorama rows use the corresponding 2:1 equirectangular dimensions and ``panoramic_shared``.


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
     - 32.33 ms
     - 4.16 ms
     - 30.93
     - 240.24
     - 7.77x
   * - 720p (1280x720)
     - 45.46 ms
     - 3.55 ms
     - 22.00
     - 281.60
     - 12.80x
   * - 1080p (1920x1080)
     - 62.83 ms
     - 5.47 ms
     - 15.91
     - 182.66
     - 11.48x
   * - 2K (2560x1440)
     - 87.18 ms
     - 8.96 ms
     - 11.47
     - 111.66
     - 9.73x
   * - 4K (3840x2160)
     - 167.45 ms
     - 14.30 ms
     - 5.97
     - 69.95
     - 11.71x
   * - 8K (7680x4320)
     - 760.37 ms
     - 36.07 ms
     - 1.32
     - 27.72
     - 21.08x

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
     - 55.54 ms
     - 43.32 ms
     - 18.00
     - 23.08
     - 1.28x
   * - 720p (1280x720)
     - 68.45 ms
     - 52.25 ms
     - 14.61
     - 19.14
     - 1.31x
   * - 1080p (1920x1080)
     - 95.61 ms
     - 55.01 ms
     - 10.46
     - 18.18
     - 1.74x
   * - 2K (2560x1440)
     - 133.99 ms
     - 60.07 ms
     - 7.46
     - 16.65
     - 2.23x
   * - 4K (3840x2160)
     - 218.88 ms
     - 72.58 ms
     - 4.57
     - 13.78
     - 3.02x
   * - 8K (7680x4320)
     - 764.85 ms
     - 197.48 ms
     - 1.31
     - 5.06
     - 3.87x

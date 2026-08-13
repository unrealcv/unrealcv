Shared-memory camera transport
==============================

.. list-table:: Open-source benchmark at 640 x 480
   :header-rows: 1
   :widths: 18 18 18 18 18 18

   * - Modality
     - TCP run 1
     - Shared run 1
     - Speedup run 1
     - Speedup run 2
     - Latency reduction
   * - Lit
     - 63.66 ms
     - 34.68 ms
     - 1.84x
     - 1.89x
     - 45.5% / 47.1%
   * - Depth
     - 33.10 ms
     - 21.02 ms
     - 1.57x
     - 1.65x
     - 36.5% / 39.2%
   * - Normal
     - 28.89 ms
     - 25.45 ms
     - 1.13x
     - 1.11x
     - 11.9% / 9.7%
   * - Object mask
     - 25.97 ms
     - 23.72 ms
     - 1.09x
     - 1.10x
     - 8.6% / 8.9%

`UnrealZoo <https://github.com/UnrealZoo>`_ Capture Transport Benchmark
========================================================================

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

Test conditions
---------------

* Windows 11; AMD Ryzen 9 5950X; NVIDIA RTX 6000D; Unreal Engine 5.7
* Local client and server; end-to-end acquisition, including complete response or mapped-byte read
* Open-source benchmark: 640 x 480, 5 warm-ups, 50 measured captures per transport and modality
* UnrealZoo benchmark: 2 warm-ups, 5 measured captures per transport and resolution

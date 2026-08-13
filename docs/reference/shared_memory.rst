Shared-memory camera transport
==============================

The Windows shared-memory transport avoids sending camera pixels through the UnrealCV TCP response. The command
returns a small JSON descriptor, and the client opens the named mapping to access the pixels.

Commands
--------

The supported routes are ``lit_shared``, ``depth_shared``, ``normal_shared``, ``object_mask_shared``, and the
``seg_shared`` alias. For example::

    vget /camera/0/lit_shared

The response contains ``name``, ``num_bytes``, ``offset_bytes``, ``shape``, ``dtype``, ``layout``, ``width``,
``height``, ``modality``, ``frame``, and ``version``. Color-like outputs use raw BGRA ``uint8`` pixels in HWC
layout. Depth uses a row-major HW array of ``float32`` values.

Python example
--------------

Use the mapping before requesting the same shared route again. A later capture can overwrite the same region::

    import json
    import mmap
    import numpy as np
    from unrealcv import Client

    client = Client(("127.0.0.1", 9000))
    client.connect()
    metadata = json.loads(client.request("vget /camera/0/lit_shared"))

    with mmap.mmap(-1, metadata["num_bytes"], tagname=metadata["name"], access=mmap.ACCESS_READ) as region:
        image = np.frombuffer(region, dtype=np.uint8).reshape(metadata["shape"]).copy()

The mapping belongs to the UnrealCV server process. Its name can change when a larger allocation is required, and
it becomes invalid when the server exits. Always use the name and version from the latest response.

Performance benchmark
---------------------

Run ``python test/benchmark_shared_memory.py`` while an UnrealCV server is listening on port 9000. The benchmark
alternates TCP and shared-memory captures for the same camera and modality, reads the complete mapping, and reports
mean, median, p95, FPS, and speedup. TCP produces encoded PNG/NPY payloads while shared memory exposes raw pixels, so
the result measures the practical acquisition paths rather than only the socket copy cost.

Measured result
~~~~~~~~~~~~~~~

The following result was measured on August 12, 2026 with the shared-memory implementation merged by PR #334
(``cdad4ec6``). The test environment was:

* Windows 11 Pro for Workstations, 64-bit, build 26200
* AMD Ryzen 9 5950X, 16 cores and 32 logical processors
* NVIDIA RTX 6000D, driver 32.0.16.1088
* 80 GB system memory
* Epic Games Unreal Engine 5.7.4, changelist 51494982
* UnrealCV camera 0 at 640 x 480 pixels
* 5 warm-up captures followed by 50 measured captures per transport and modality
* Local client and server on ``127.0.0.1:9000``

Each timing is end-to-end from the Python request until the client has acquired all image bytes. The TCP path includes
receiving the encoded PNG or NPY response. The shared-memory path includes parsing the JSON descriptor, opening the
named mapping, and copying the complete raw mapping into Python memory. Tests were run twice with the modality order
reversed in the second run to check for order-dependent bias.

.. list-table:: End-to-end mean acquisition latency and speedup at 640 x 480
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

The reverse-order run measured shared-memory means of 34.26 ms for lit, 21.29 ms for depth, 25.03 ms for normal,
and 23.05 ms for object mask. The corresponding TCP means were 64.70 ms, 35.03 ms, 27.73 ms, and 25.30 ms.

At this resolution, each raw shared-memory image is 1,228,800 bytes. The first-run TCP payloads were approximately
275 KB for lit PNG, 1,228,880 bytes for depth NPY, 27 KB for normal PNG, and 8 KB for object-mask PNG. Lit and depth
therefore show the largest reduction in end-to-end latency. Normal and object mask are already highly compressible,
so copying the fixed-size raw mapping leaves a smaller but repeatable improvement.

The implementation is currently Windows-only. Remote clients cannot open a mapping from another machine and must
continue using TCP or file output.

UnrealZoo Capture Transport Benchmark
======================================

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

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

UnrealCV Dev For UnrealZoo 拿图速度benchmark
==============================================

This section is a closed-build measurement of the UnrealZoo distribution. It uses the same end-to-end definition as
the benchmark above: the TCP path receives the complete BMP response, while the shared-memory path parses the JSON
descriptor, opens the named mapping, and reads all mapped bytes. The raw report is committed as
``test/unrealzoo_capture_transport_20260813.json`` and can be regenerated against a running server with::

    python test/benchmark_unrealzoo_capture_transport.py --iterations 5 --warmup 2 \
        --output test/unrealzoo_capture_transport.json

The run used the closed ``HUAWEI_Project.exe`` built on August 13, 2026, local TCP (``127.0.0.1:9000``), camera 0,
five measured captures after two warm-ups, and the same machine/UE 5.7 environment as the shared-memory result above.
The standard-camera rows use ``vget /camera/0/lit bmp`` versus ``lit_shared``. Panorama rows use the corresponding
2:1 equirectangular dimensions and ``panoramic_shared``.

.. list-table:: Closed-build standard camera: mean end-to-end latency
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

.. list-table:: Closed-build panorama: mean end-to-end latency
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

The ordinary-camera shared-memory path reduces mean latency by 87.1%--95.3%. Panorama gains are smaller at low
resolution because panorama rendering dominates acquisition; at 8K the reduction is 74.2%. These are one-machine,
five-sample engineering measurements, not a claim that shared memory increases the renderer's frame rate.

横向能力对比（非同机实测）
--------------------------

.. list-table:: Public camera-acquisition interface comparison
   :header-rows: 1
   :widths: 20 28 24 28

   * - Simulator
     - Public camera interface
     - Documented return path
     - Equivalent to this benchmark
   * - UnrealCV Dev For UnrealZoo
     - ``vget /camera/<id>/lit_shared`` and ``panoramic_shared``
     - Windows named shared-memory descriptor plus raw mapped bytes
     - Yes; measured above with a complete client-side mapping read
   * - CARLA
     - Camera sensor callback
     - ``carla.Image.raw_data`` is exposed to the Python client
     - No standard named-shared-memory image route documented
   * - AirSim
     - ``simGetImages`` RPC
     - RPC response list containing ``ImageResponse`` data
     - No standard named-shared-memory image route documented
   * - SimWorld
     - Version and deployment specific
     - No stable public camera transport reference was identified for this comparison
     - Not benchmarked; a versioned API and runnable build are required

CARLA's sensor reference and AirSim's API reference describe the two client-returned image interfaces above. This is
an interface comparison, not an FPS ranking: it does not make a performance claim for CARLA, AirSim, or SimWorld. A
fair cross-simulator throughput table requires pinning simulator version, renderer settings, camera resolution, image
encoding, client language, and whether the measurement includes a full byte copy; without those controls, published
FPS figures are not comparable to this benchmark.

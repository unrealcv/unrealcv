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

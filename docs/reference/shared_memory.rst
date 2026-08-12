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

The implementation is currently Windows-only. Remote clients cannot open a mapping from another machine and must
continue using TCP or file output.

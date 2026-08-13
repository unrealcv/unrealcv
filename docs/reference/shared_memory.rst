Shared-memory transport
=======================

The UnrealCV shared-memory implementation draws on the
`SPEAR project paper <https://arxiv.org/abs/2607.06701>`_ and
`source repository <https://github.com/spear-sim/spear>`_.

Performance benchmark
---------------------

Run ``python test/benchmark_shared_memory.py`` while an UnrealCV server is listening on port 9000. The benchmark
alternates TCP and shared-memory captures for the same camera and modality, reads the complete mapping, and reports
mean, median, p95, FPS, and speedup. TCP produces encoded PNG/NPY payloads while shared memory exposes raw pixels, so
the result measures the practical acquisition paths rather than only the socket copy cost.


Measured result
~~~~~~~~~~~~~~~

The following result was measured on August, 2026 with the shared-memory implementation merged by PR #334.
The test environment was:


* Windows 11
* AMD Ryzen 9 5950X, 16 cores and 32 logical processors
* NVIDIA RTX 6000D
* Epic Games Unreal Engine 5.7.4
* UnrealCV camera 0 at 640 x 480 pixels
* 5 warm-up captures followed by 50 measured captures per transport and modality

Each timing is end-to-end from the Python request until the client has acquired all image bytes. The TCP path includes
receiving the encoded PNG or NPY response. The shared-memory path includes parsing the JSON descriptor, opening the
named mapping, and copying the complete raw mapping into Python memory. Tests were run twice with the modality order
reversed in the second run to check for order-dependent bias.


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

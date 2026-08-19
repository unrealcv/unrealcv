# UnrealCV Dev for [UnrealZoo](https://github.com/UnrealZoo)

These continuously developed features are currently provided and tested first
in [UnrealZoo](https://github.com/UnrealZoo). They are not part of the
open-source UnrealCV command contract unless they also appear in the main
Command System reference.

The feature pages in the UnrealCV Dev for UnrealZoo section document the
development-only capture, recording, annotation, and runtime object workflows.
The PAK workflow is intentionally documented outside this overview so the
overview remains focused on runtime capabilities.

- Improved capture performance for common modalities such as `lit`, `mask`, and `depth`.
  - Since December 2025, frame throughput has been steadily improved, especially around the `BaseCamSensor` path.
  - In test cases, `lit` capture throughput improved from roughly `15 fps` to `20 fps`.

- Updated camera ID support to remain compatible with existing legacy camera addressing while also providing the newer stable `CID` format for long-term use in scripts and configurations.
  - Existing Python camera APIs remain backward compatible.
  - New Python APIs expose stable `CID-*` camera identifiers explicitly:
    - `get_camera_list_cid()`
    - `get_camera_id_map()`
  - Legacy camera discovery remains available through:
    - `get_camera_list_legacy()`

- Added annotation command support for scene and actor labeling workflows. See
  :doc:`reference/annotation-system` for the command and Python API details.

- Added object spawning from asset paths via `spawn_from_path`. See
  :doc:`reference/object-spawning-from-path` for the path format and examples.

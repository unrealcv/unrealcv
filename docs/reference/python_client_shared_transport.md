# Python client automatic Shared Memory transport

This table defines which Python functions automatically select Windows Shared
Memory when the connected server advertises the corresponding command.

| Python function | Automatic Shared command | Legacy fallback | Actual return contract |
| --- | --- | --- | --- |
| `UnrealCv_API.get_image` | `/camera/{id}/{lit\|normal\|object_mask\|seg}_shared` | `/camera/{id}/{viewmode} {bmp\|png}` | Same decoded NumPy image |
| `UnrealCv_API.get_depth` | `/camera/{id}/depth_shared` | `/camera/{id}/depth npy` | Same decoded float32 NumPy depth |
| `UnrealCv_API.get_image_multicam` | Inherits `get_image` | Per-camera TCP fallback | Same list of images |
| `UnrealCv_API.get_image_multimodal` | Inherits `get_image`/`get_depth` | Per-modality TCP fallback | Same concatenated NumPy array |
| `UnrealCv_API.get_img_batch` | Inherits `get_image` | Per-image TCP fallback | Same populated batch dictionary |
| `UnrealCvPlusAPI.get_scene_occupancy` | `/scene/occupancy_shared ...` | `/scene/occupancy ...` | Same decoded bool NumPy grid |
| `UnrealCvPlusAPI.get_scene_occupancy_region` | `/scene/occupancy_shared_region ...` | `/scene/occupancy_region npy ...` | Same decoded bool NumPy region |
| `UnrealCvPlusAPI.get_camera_panoramic_frame` | `/camera/{id}/panoramic_shared [uint] [uint]` | None: legacy command only writes a file | Decoded NumPy image |
| `UnrealCvPlusAPI.get_camera_panoramic_normal_frame` | `/camera/{id}/panoramic/normal_shared [uint] [uint]` | None: legacy command only writes a file | Decoded NumPy image |
| `UnrealCvPlusAPI.get_camera_panoramic_mask_frame` | `/camera/{id}/panoramic/mask_shared [uint] [uint]` | None: legacy command only writes a file | Decoded NumPy image |
| `UnrealCvPlusAPI.get_camera_panoramic_depth_frame` | `/camera/{id}/panoramic/depth_shared [uint] [uint]` | None: legacy command only writes a file | Decoded float32 NumPy depth |
| `UnrealCvPlusAPI.get_camera_mqrc_lit_frame` | `/camera/{id}/mqrc/lit_shared` | None: legacy command only writes a file | Decoded NumPy image |
| `UnrealCvPlusAPI.get_camera_lidar_frame` | `/camera/{id}/lidar_shared` | None: legacy LiDAR command has a different contract | Raw `N x 4` float32 NumPy point cloud |
| `UnrealCvPlusAPI.get_camera_mqrc_panoramic_frame` | `/camera/{id}/mqrc/panoramic_shared [uint] [uint] [uint]` | None: legacy command only writes a file | Decoded NumPy image |

The selection decision is owned by `ApiVersionManager`. `Client.request`
executes the selected command and makes Shared Memory transparent by converting
the mapping contents back to the legacy BMP, PNG, or NPY byte format before the
existing decoder runs.

The `*_frame` APIs intentionally require an advertised Shared command. The
legacy panoramic and MQRC commands take a filename and therefore cannot provide
the same in-memory return contract; those file APIs remain available separately.

The following APIs remain TCP/file APIs because a Shared Memory response would
change their contract:

- `UnrealCvPlusAPI.capture_panoramic`
- `UnrealCvPlusAPI.capture_panoramic_normal`
- `UnrealCvPlusAPI.capture_panoramic_mask`
- `UnrealCvPlusAPI.capture_panoramic_depth`

The machine-readable version of this table is
`python_client_shared_transport.json`.

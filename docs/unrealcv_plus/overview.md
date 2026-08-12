# UnrealCV+ Plugin

Summary of notable UnrealCV+ changes in v3.0 stage.

- Added runtime `PAK` mounting support for packaged applications, making it easier to extend content without rebuilding the whole project.
  - Supports mount, unmount, mounted pak listing, mount-state checks, file enumeration, asset enumeration, asset rescanning, and dynamic asset loading/registration.
  - Python APIs:
    - `mount_pak()`
    - `unmount_pak()`
    - `get_mounted_paks()`
    - `is_pak_mounted()`
    - `get_pak_files()`
    - `get_pak_assets_in_pak()`
    - `scan_pak_assets()`
    - `load_pak_asset()`
    - `get_pak_assets()`
    - `register_pak_assets()`
  - See unrealzoo website: `Home`->`Document`->`Import Custom Assets` for more information.

- Added panoramic camera support for 360-degree equirectangular image generation.
  - Supports per-camera panorama cube resolution and direct panorama export to file.
  - Python APIs:
    - `set_camera_panoramic_resolution()`
    - `capture_panoramic()`

- Added a faster C++ video recording pipeline, improving recording efficiency and making large-scale capture workflows more practical.
  - Supports direct recording from a camera or actor, configurable output directory, frame rate, duration, and selected recording channels.
  - Python APIs:
    - `start_simple_recording()`: start a recording job with output path, FPS, duration, and selected channels such as `lit` or `mask`.
    - `stop_recording()`: stop an active recording job for the target camera or capture actor.
    - `is_recording()`: check if a camera is currently recording.
    - `get_use_movie_quality_rendering()` / `set_use_movie_quality_rendering()`: query or change the global movie-quality rendering switch used by the recording pipeline.
    - `get_record_via_viewport()` / `set_record_via_viewport()`: query or change the global switch for whether recording uses the viewport capture path.
    - `get_warmup_frames()` / `set_warmup_frames()`: query or configure the global number of warmup frames rendered before recording starts.
    - `get_paused_tick_interval()` / `set_paused_tick_interval()`: query or configure the global paused tick interval in seconds.
    - `get_record_add_timestamp()` / `set_record_add_timestamp()`: query or control whether an active capture actor appends a timestamp suffix to recorded outputs.
    - `get_recording_paused()` / `set_recording_paused()`: query or control whether an active recording session is paused.

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

- Added annotation command support for scene and actor labeling workflows.
  - Supports annotating a single actor, annotating the whole world, clearing world annotation, enabling/disabling annotation cache, and clearing cached annotation components.
  - Python APIs:
    - `annotate_object()`
    - `annotate_world()`
    - `clear_world_annotation()`
    - `set_annotation_cache_enabled()`
    - `clear_annotation_cache()`

- Added object spawning from asset paths via `spawn_from_path`.
  - This complements the older class-based spawn flow and is better suited for runtime content referenced by full asset path.
  - Python APIs:
    - `spawn_object_from_path()`
  - `set_new_obj()` now gives a clearer hint when the input looks like an asset path, and can automatically retry via `spawn_object_from_path()`.


## PAK Mounting

The PAK mounting system allows you to load additional content at runtime without rebuilding the project. This is useful for dynamically extending your scene with external assets.

### Usage

The PAK system supports:
- Mounting/unmounting PAK files
- Listing mounted PAKs and checking mount status
- Enumerating files and assets within PAKs
- Scanning and registering assets for use

### Example

```python
import unrealcv

# Connect to the server
client = unrealcv.Client('127.0.0.1', 9000)

# Mount a PAK file (specify the full path to the .pak file)
client.mount_pak('C:/Content/ExtraAssets.pak', pak_order=0)

# Check if a PAK is mounted
if client.is_pak_mounted('C:/Content/ExtraAssets.pak'):
    print("PAK is mounted successfully")

# List all mounted PAKs
mounted_paks = client.get_mounted_paks()
for pak in mounted_paks:
    print(f"Mounted: {pak}")

# List files inside the PAK
files = client.get_pak_files('C:/Content/ExtraAssets.pak')
print(f"Files in PAK: {len(files)}")

# Get Unreal assets from the PAK
assets = client.get_pak_assets_in_pak('C:/Content/ExtraAssets.pak')
print(f"Assets in PAK: {len(assets)}")

# Scan assets from a mounted PAK mount point
client.scan_pak_assets('/Game/ExtraAssets', force_rescan=True)

# Load an asset from the PAK
asset_data = client.load_pak_asset('/Game/ExtraAssets/MyActor.MyActor')

# Register assets into the UnrealCV asset pool for spawning
client.register_pak_assets('/Game/ExtraAssets', 'Props')

# Unmount when done
client.unmount_pak('C:/Content/ExtraAssets.pak')
```

---

## Panoramic Camera

Panoramic camera capture generates 360-degree equirectangular images from a point in your scene, useful for VR previews and virtual tours.

### Usage

- Configure per-camera cube map resolution for quality control
- Capture panoramic images directly to file
- Optional output dimension specification

### Example

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)

# Set panoramic camera resolution (cube map face resolution)
# Higher values = better quality but slower capture
client.set_camera_panoramic_resolution('Camera1', 1024)

# Capture a panoramic image to file
# The output will be an equirectangular projection (2:1 aspect ratio)
output_path = 'C:/captures/panoramic_01.jpg'
client.capture_panoramic('Camera1', output_path)

# Capture with custom output dimensions
client.capture_panoramic('Camera1', 'C:/captures/pano_360.png', width=4096, height=2048)

# You can also capture to different formats (JPG, PNG, EXR)
client.capture_panoramic('Camera1', 'C:/captures/panoramic.exr')
```

---

## Video Recording Pipeline

The C++ video recording pipeline provides high-performance capture with configurable settings for large-scale generation workflows.

### Usage

- `start_simple_recording()`: Start recording with output path, FPS, duration, and channels
- `stop_recording()`: Stop an active recording
- `is_recording()`: Check if a camera is currently recording
- Global settings for movie-quality rendering, viewport capture, warmup frames, and tick intervals

### Example

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)

# Configure global recording settings
client.set_use_movie_quality_rendering(False) # Lumen + high-quality settings
client.set_record_via_viewport(True)          # Use main viewport
client.set_warmup_frames(10)                  # Render 10 frames before recording starts

# Start recording a 5-second clip at 30 FPS
# Record options can be: 'lit', 'mask', 'depth', 'normal', 'flow', etc.
cam_id = '1'
output_folder = 'C:/recordings/scene01'
fps = 30
duration = 5.0
record_options = ['lit', 'mask']  # Record both RGB and segmentation

client.start_simple_recording(cam_id, output_folder, fps, duration, record_options)

# Optionally pause/resume the recording
client.set_recording_paused(cam_id, paused=True)
# ... do something else ...
client.set_recording_paused(cam_id, paused=False)

# Stop the recording when complete
client.stop_recording(cam_id)

# Wait for recording to complete using a while loop
import time
while client.is_recording(cam_id):
    print("Recording in progress...")
    time.sleep(0.5)  # Check every 500ms
print("Recording completed!")

# Check recording status
is_paused = client.get_recording_paused(cam_id)
```

---

## Annotation System

The annotation system provides scene labeling for semantic segmentation and object detection training workflows.

### Usage

- `annotate_object()`: Label a single actor
- `annotate_world()`: Annotate all actors in the scene
- `clear_world_annotation()`: Remove all annotations
- Cache management for performance optimization

### Example

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)

# Annotate a specific actor by name
client.annotate_object('Chair_01')

# Annotate all actors in the current world
client.annotate_world()

# Clear all annotations
client.clear_world_annotation()

# Enable annotation cache for better performance
client.set_annotation_cache_enabled(True)

# Clear the annotation cache when done
client.clear_annotation_cache()
```

---

## Object Spawning from Path

Spawn assets directly using full asset paths, which is more convenient for runtime content loading than class-based spawning.

### Usage

- `spawn_object_from_path()`: Spawn from full asset path with auto-annotation
- Automatic retry support when input looks like a path

### Example

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)

# Spawn an object from its full asset path
# Format: /Game/Path/To/Asset.AssetName
asset_path = '/Game/Props/Chair_Mesh.Chair_Mesh'
obj_name = 'MySpawnedChair'

# With auto-annotation (default)
client.spawn_object_from_path(asset_path, obj_name)

# Without auto-annotation (stable)
client.spawn_object_from_path(asset_path, obj_name, annotate=False)

# set_new_obj() automatically detects path-like input and retries with spawn_object_from_path()
# This provides backward compatibility while supporting direct path spawning
client.set_new_obj('/Game/Props/Table.Table', 'MyTable')
```

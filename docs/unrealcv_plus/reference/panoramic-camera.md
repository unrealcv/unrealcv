# Panoramic Camera

UnrealCV Dev For UnrealZoo can capture a 360-degree equirectangular image from
a camera. The output uses a 2:1 panorama layout.

## Python API

- `set_camera_panoramic_resolution(camera_id, cube_face_resolution)` sets the
  cube-map face resolution used for the capture.
- `capture_panoramic(camera_id, output_path, width=None, height=None)` writes
  the panorama to an image file.

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)
client.set_camera_panoramic_resolution('Camera1', 1024)
client.capture_panoramic('Camera1', 'C:/captures/panoramic.png')
client.capture_panoramic(
    'Camera1', 'C:/captures/panoramic_4k.png', width=4096, height=2048)
```

Higher cube-face resolutions improve detail and increase capture cost. Use a
`.png`, `.jpg`, or `.exr` output path according to the required image format.

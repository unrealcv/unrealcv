# Video Recording Pipeline

The UnrealCV Dev For UnrealZoo C++ recording path is intended for repeatable,
large-scale capture. It can record selected channels such as `lit`, `mask`,
`depth`, `normal`, and `flow`.

## Python API

- `start_simple_recording(camera_id, output_dir, fps, duration, channels)`
- `stop_recording(camera_id)`
- `is_recording(camera_id)`
- `set_use_movie_quality_rendering(value)`
- `set_record_via_viewport(value)`
- `set_warmup_frames(count)`
- `set_paused_tick_interval(seconds)`
- `set_recording_paused(camera_id, paused)`

```python
import time
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)
client.set_use_movie_quality_rendering(False)
client.set_record_via_viewport(True)
client.set_warmup_frames(10)

camera_id = '1'
client.start_simple_recording(
    camera_id,
    'C:/recordings/scene01',
    30,
    5.0,
    ['lit', 'mask'])

while client.is_recording(camera_id):
    time.sleep(0.5)
```

Recording settings that affect temporal state should be configured before the
recording starts. The output directory is created by the runtime capture path.

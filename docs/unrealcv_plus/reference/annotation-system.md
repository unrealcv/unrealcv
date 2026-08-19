# Annotation System

The UnrealCV Dev For UnrealZoo annotation commands apply semantic labels to
actors or to the complete world. They support segmentation and object-detection
capture workflows.

## Python API

- `annotate_object(actor_name)`
- `annotate_world()`
- `clear_world_annotation()`
- `set_annotation_cache_enabled(enabled)`
- `clear_annotation_cache()`

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)
client.annotate_object('Chair_01')
client.annotate_world()
client.set_annotation_cache_enabled(True)
client.clear_annotation_cache()
client.clear_world_annotation()
```

Use the cache when the same semantic setup is reused across many frames. Clear
the cache after changing actor labels or material metadata.

# Object Spawning from Path

`spawn_from_path` creates an object directly from a full Unreal asset path. It
is useful when the caller has a package path rather than a loaded `UClass`.

## Python API

`spawn_object_from_path(asset_path, object_name, annotate=True)` accepts paths
in the form `/Game/Folder/Asset.Asset`.

```python
import unrealcv

client = unrealcv.Client('127.0.0.1', 9000)
client.spawn_object_from_path(
    '/Game/Props/Chair_Mesh.Chair_Mesh',
    'MySpawnedChair')
client.spawn_object_from_path(
    '/Game/Props/Table.Table',
    'MyTable',
    annotate=False)
```

`set_new_obj()` can detect path-like input and retry through
`spawn_object_from_path()` for compatibility with older scripts. The asset
must be available to the running Unreal process and resolve to a spawnable
actor or blueprint.

Updated Server from UnrealCV

## New Features
 - Set object's mobility by `vset /object/[str]/object_mobility [str]`
 - Set object's collision by `vset /object/[str]/collision [str]`
 - Set object's physics by `vset /object/[str]/physics [str]`
 - Spawn blueprint asset by `vset /objects/spawn_bp_asset [str] [str]`
   - The first string is the reference to that blueprint. For example, `Blueprint'/Game/CityDatabase/blueprints/BP_Building_01.BP_Building_01_C'` 
   - The second string is the ID in the unreal engine. For example, `Building_01`
 - Manually collect garbage in the memory to clean unused reference by `vset /action/clean_garbage`
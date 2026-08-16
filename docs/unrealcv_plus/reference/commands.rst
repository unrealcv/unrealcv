UnrealCV Dev For `UnrealZoo <https://github.com/UnrealZoo>`_ command reference
=============================================

Commands on this page are development capabilities tested and currently
provided by `UnrealZoo <https://github.com/UnrealZoo>`_. They are not part of the open-source UnrealCV command
contract unless they also appear in :doc:`../../reference/commands`.

Do not assume availability from the Python method alone. Query
``vget /unrealcv/commands`` at runtime and gate optional workflows on the exact
command template returned by the server.

Agent navigation
----------------

``vget /agent/[str]/nav/status``
    Get agent navigation status

``vset /agent/[str]/nav/goto [float] [float] [float]``
    Navigate agent to a specific position (x y z)

``vset /agent/[str]/nav/start [float]``
    Start autonomous navigation for an agent with specified radius

``vset /agent/[str]/nav/stop``
    Stop agent navigation

Animation
---------

``vget /animation/smooth_random/play_rate_multiplier``
    Get the global Smooth Random Sequence Player play-rate multiplier.

``vget /animation/soma_glb/status [str]``
    Get SOMA GLB playback status for an actor.

``vset /animation/smooth_random/play_rate_multiplier [float]``
    Set the non-negative global Smooth Random Sequence Player play-rate multiplier.

``vset /animation/soma_glb/apply [str] [Anything]``
    Apply a SOMA GLB animation to a MetaHuman actor. Args: actor name, absolute GLB path.

``vset /animation/soma_glb/stop [str]``
    Stop SOMA GLB playback and restore the actor's previous body animation class.

Annotation
----------

``vset /annotation/cache/clear``
    Clear annotation component cache

``vset /annotation/cache/enable [uint]``
    Enable or disable annotation component cache [0|1]

``vset /annotation/object/[str]``
    Annotate a single actor by object id

``vset /annotation/world``
    Annotate the current world

``vset /annotation/world/clear``
    Remove world annotation

Bs
--

``vget /bs/all_ever_existed_uid``
    Get active and trashed BVRSim object UIDs

``vget /bs/all_uid``
    Get active BVRSim object UIDs

``vget /bs/dt``
    Get BVRSim fixed simulation dt

``vget /bs/sim_time``
    Get BVRSim simulation time

``vset /bs/dt [float]``
    Set BVRSim fixed simulation dt

Bvr Sim
-------

``vget /bvr_sim/status``
    Get BVRSim bootstrap status

``vset /bvr_sim/reset``
    Reset BVRSim world state

``vset /bvr_sim/step [float]``
    Step BVRSim by delta seconds

Camera
------

The physical camera commands are described in detail in
:doc:`cine-camera`. The cinematic path is disabled by default, and clients
should detect these development commands at runtime.

``vget /camera/[camera_id]/cine``
    Get physical cinematic camera settings as JSON.

``vget /camera/[camera_id]/cine/enabled``
    Get whether the physical cinematic camera path is enabled (``0`` or ``1``).

``vget /camera/[camera_id]/cine/intrinsics``
    Get image dimensions, camera intrinsics, projection offsets, fields of view,
    and the 4x4 projection matrix as JSON.

``vset /camera/[camera_id]/cine/enabled [uint]``
    Enable or disable the physical cinematic camera path.

``vset /camera/[camera_id]/cine/filmback [float] [float] [float] [float]``
    Set filmback width, height, horizontal offset, and vertical offset in
    millimeters.

``vset /camera/[camera_id]/cine/lens [float] [float]``
    Set physical focal length in millimeters and aperture in f-stops.

``vset /camera/[camera_id]/cine/lens_settings [float] [float] [float] [float] [float] [float] [uint]``
    Set focal-length limits, f-stop limits, minimum focus distance, squeeze
    factor, and diaphragm blade count.

``vset /camera/[camera_id]/cine/focus [float]``
    Set manual focus distance in centimeters.

``vset /camera/[camera_id]/cine/focus_mode [str] [uint] [float] [float]``
    Set focus mode, smoothing state, smoothing speed, and focus offset.

``vset /camera/[camera_id]/cine/focus_tracking [str] [float] [float] [float]``
    Set the tracking-focus actor and relative offset in centimeters.

``vset /camera/[camera_id]/cine/crop [float] [float] [uint] [uint]``
    Set crop aspect ratio, overscan, crop-overscan state, and overscan
    resolution-scaling state.

``vset /camera/[camera_id]/cine/near_clip [uint] [float]``
    Enable or disable a custom near clipping plane and set its distance in
    centimeters.

``vset /camera/[camera_id]/cine/exposure [float] [float] [uint]``
    Set ISO, shutter-speed reciprocal, and physical-exposure state.

``vget /camera/[camera_id]/depth/exp``
    Get depth hue exponent

``vget /camera/[camera_id]/depth/max_distance``
    Get depth hue maximum distance

``vget /camera/[camera_id]/depth/min_distance``
    Get depth hue minimum distance

``vget /camera/[camera_id]/depth/use_exp``
    Get whether depth hue uses exponential mapping

``vget /camera/[camera_id]/dual_depth [str] [str]``
    Capture foreground and background depth images to PNG or JPG files

``vget /camera/[camera_id]/id``
    Get camera legacy index, stable ID and sensor name as JSON

``vget /camera/[camera_id]/mqrc/lit [str]``
    Get png data from movie quality render component

``vget /camera/[camera_id]/mvrc/enabled``
    Get whether main viewport render component is enabled

``vget /camera/[camera_id]/mvrc/lit [str]``
    Get png data from main viewport render component

``vget /camera/[camera_id]/oneobjlit [str] [str]``
    oneobjlit bmp object_id

``vget /camera/[camera_id]/oneobjlit_legacy [str] [str]``
    Legacy ShowOnlyList oneobjlit image object_id

``vget /camera/[camera_id]/oneobjmask [str] [str]``
    oneobjmask bmp object_id

``vget /camera/[camera_id]/panoramic [str]``
    Capture panoramic equirectangular image to file

``vget /camera/[camera_id]/panoramic [str] [uint] [uint]``
    Capture panoramic equirectangular image with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/depth [str]``
    Capture panoramic equirectangular depth preview image to file

``vget /camera/[camera_id]/panoramic/depth [str] [uint] [uint]``
    Capture panoramic equirectangular depth preview image with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/depth_shared``
    Get panoramic equirectangular depth float32 data through shared memory and return JSON metadata

``vget /camera/[camera_id]/panoramic/depth_shared [uint] [uint]``
    Get panoramic equirectangular depth float32 data through shared memory with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/mask [str]``
    Capture panoramic equirectangular object mask image to file

``vget /camera/[camera_id]/panoramic/mask [str] [uint] [uint]``
    Capture panoramic equirectangular object mask image with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/mask_shared``
    Get panoramic equirectangular object mask BGRA8 data through shared memory and return JSON metadata

``vget /camera/[camera_id]/panoramic/mask_shared [uint] [uint]``
    Get panoramic equirectangular object mask BGRA8 data through shared memory with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/normal [str]``
    Capture panoramic equirectangular normal image to file

``vget /camera/[camera_id]/panoramic/normal [str] [uint] [uint]``
    Capture panoramic equirectangular normal image with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic/normal_shared``
    Get panoramic equirectangular normal BGRA8 data through shared memory and return JSON metadata

``vget /camera/[camera_id]/panoramic/normal_shared [uint] [uint]``
    Get panoramic equirectangular normal BGRA8 data through shared memory with custom resolution [width, height]

``vget /camera/[camera_id]/panoramic_shared``
    Get panoramic equirectangular BGRA8 data through shared memory and return JSON metadata

``vget /camera/[camera_id]/panoramic_shared [uint] [uint]``
    Get panoramic equirectangular BGRA8 data through shared memory with custom resolution [width, height]

``vget /camera/[camera_id]/render_in_main_renderer``
    Get whether sensors render in main renderer for optimization (0 or 1)

``vget /camera/[camera_id]/use_fast_capture``
    Get fast capture mode status (0 or 1)

``vset /camera/[camera_id]/depth/exp [float]``
    Set depth hue exponent

``vset /camera/[camera_id]/depth/max_distance [float]``
    Set depth hue maximum distance

``vset /camera/[camera_id]/depth/min_distance [float]``
    Set depth hue minimum distance

``vset /camera/[camera_id]/depth/use_exp [uint]``
    Set whether depth hue uses exponential mapping

``vset /camera/[camera_id]/lookat_object_auto [str] [float] [float] [float] [float]``
    Automatically position camera to view target object [obj_name] [min_dist] [max_dist] [min_angle] [max_angle]

``vset /camera/[camera_id]/panoramic/resolution [uint]``
    Set panoramic cubemap resolution for the camera

``vset /camera/[camera_id]/render_in_main_renderer [uint]``
    Set whether sensors render in main renderer (0=disabled, 1=enabled)

``vset /camera/[camera_id]/use_fast_capture [uint]``
    Set fast capture mode (0=disabled, 1=enabled)

Camera collection
-----------------

``vget /cameras/ids``
    List all cameras with legacy indices and stable IDs as JSON

Cameras Cid
-----------

``vget /cameras_CID``
    List all cameras with new format CIDs (Camera UUID)

Cameras Legacy
--------------

``vget /cameras_legacy``
    List all sensors in the scene (old format)

Recording
---------

``vget /captureactor/[camera_id]/is_recording``
    Check if a camera is currently recording: vget /captureactor/[id]/is_recording

``vget /captureactor/[camera_id]/paused``
    Get bPaused state for camera: vget /captureactor/[id]/paused

``vset /captureactor/[camera_id]/paused [uint]``
    Set bPaused state for camera: vset /captureactor/[id]/paused [0/1]

``vset /captureactor/[camera_id]/record [str] [uint] [float]``
    Start simple recording: vset /captureactor/[id]/record [output_folder] [fps] [duration_seconds] [record_options]

``vset /captureactor/[camera_id]/record [str] [uint] [float] [str]``
    Start simple recording: vset /captureactor/[id]/record [output_folder] [fps] [duration_seconds] [record_options]

``vset /captureactor/[camera_id]/stop_record``
    Stop recording for a camera: vset /captureactor/[id]/stop_record

Editor
------

``vset /editor/start_standalone_pie``
    Start Standalone PIE in a separate process

Lighting
--------

``vget /light/directional/castdeepshadow``
    Get DirectionalLight cast deep shadow

``vget /light/directional/intensity``
    Get DirectionalLight intensity

``vget /light/skylight/intensity``
    Get SkyLight intensity

``vset /light/directional/castdeepshadow [bool]``
    Set DirectionalLight cast deep shadow

``vset /light/directional/intensity [float]``
    Set DirectionalLight intensity

``vset /light/skylight/intensity [float]``
    Set SkyLight intensity

LLM
---

``vget /llm/config``
    Get current LLM config

``vget /llm/request/[str]/result``
    Get async LLM request result

``vget /llm/request/[str]/status``
    Get async LLM request status

``vset /llm/chat [Anything]``
    Start an async chat completion request with the remainder as user prompt

``vset /llm/chat_json [Anything]``
    Start an async request with raw JSON body against the configured endpoint

``vset /llm/config/api_key [str]``
    Set API key for OpenAI-compatible endpoint

``vset /llm/config/base_url [str]``
    Set chat completion endpoint URL

``vset /llm/config/model [str]``
    Set default model name

``vset /llm/config/wire_api [str]``
    Set wire API type: chat_completions or responses

MetaHuman
---------

``vget /metahuman/all_paths``
    Get all MetaHuman blueprint paths (scans AssetRegistry and saves cache)

``vget /metahuman/cache_path``
    Get the path to MetaHuman cache file

``vget /metahuman/filter_batch``
    Filter batch generated MetaHumans from cache

``vget /metahuman/head_aim_location [str]``
    Get preferred MetaHuman head aim location for actor. Optional args: target bone name, target height ratio.

``vget /metahuman/head_aim_location [str] [str]``
    Get preferred MetaHuman head aim location for actor. Optional args: target bone name, target height ratio.

``vget /metahuman/head_aim_location [str] [str] [float]``
    Get preferred MetaHuman head aim location for actor. Optional args: target bone name, target height ratio.

``vget /metahuman/parametric/rig_status [str]``
    Get rigging status. Args: [CharacterPath]

``vset /metahuman/parametric/assemble [str] [str] [str]``
    Assemble a rigged MetaHuman. Args: [CharacterPath] [OutputPath] [QualityLevel]

``vset /metahuman/parametric/body [str] [str] [float] [float] [float]``
    Update body parameters. Args: [CharacterPath] [BodyType] [HeightCm] [ChestCm] [WaistCm]

``vset /metahuman/parametric/create [str] [str] [str] [float] [float] [float] [str] [str]``
    Create and start rigging a MetaHuman. Args: [CharacterName] [OutputPath] [BodyType] [HeightCm] [ChestCm] [WaistCm] [HairPath] [Clothing1|Clothing2|...]

``vset /metahuman/parametric/wardrobe [str] [str] [str]``
    Apply hair and wardrobe. Args: [CharacterPath] [HairPath] [Clothing1|Clothing2|...]

``vset /metahuman/update_cache``
    Scan AssetRegistry and update MetaHuman cache file

Misc
----

``vget status``
    Get the status of UnrealCV plugin

``vreflect [str] call_json [str] [Anything]``
    Reflection bridge for listing functions/properties and getting/setting properties

Movie-quality rendering
-----------------------

``vget /mqrc/antialiasing``
    Get anti-aliasing method

``vget /mqrc/auto_exposure_max_brightness``
    Get auto-exposure maximum brightness

``vget /mqrc/auto_exposure_min_brightness``
    Get auto-exposure minimum brightness

``vget /mqrc/depth_of_field_scale``
    Get depth of field scale

``vget /mqrc/exposure_bias``
    Get exposure bias

``vget /mqrc/exposure_method``
    Get exposure method

``vget /mqrc/lumen_final_gather_lighting_update_speed``
    Get Lumen final gather lighting update speed

``vget /mqrc/lumen_quality``
    Get Lumen quality settings (scene_quality gather_quality)

``vget /mqrc/motion_blur``
    Get motion blur amount

``vget /mqrc/override_auto_exposure_max_brightness``
    Get auto-exposure maximum brightness override flag

``vget /mqrc/override_auto_exposure_min_brightness``
    Get auto-exposure minimum brightness override flag

``vget /mqrc/override_depth_of_field_scale``
    Get depth of field scale override flag

``vget /mqrc/override_exposure_bias``
    Get exposure bias override flag

``vget /mqrc/override_exposure_method``
    Get exposure method override flag

``vget /mqrc/override_lumen_final_gather_lighting_update_speed``
    Get override flag for Lumen final gather lighting update speed

``vget /mqrc/override_lumen_final_gather_quality``
    Get Lumen final gather quality override flag

``vget /mqrc/override_lumen_scene_lighting_quality``
    Get Lumen scene lighting quality override flag

``vget /mqrc/override_motion_blur``
    Get motion blur amount override flag

``vget /mqrc/render_immediately``
    Get render immediately mode (true/false)

``vget /mqrc/screen_percentage``
    Get screen percentage (1.0 = 100%)

``vget /mqrc/screen_percentage_method``
    Get primary screen percentage method

``vset /mqrc/antialiasing [str]``
    Set anti-aliasing method: fxaa, temporal_aa, tsr, none

``vset /mqrc/auto_exposure_max_brightness [float]``
    Set auto-exposure maximum brightness

``vset /mqrc/auto_exposure_min_brightness [float]``
    Set auto-exposure minimum brightness

``vset /mqrc/capture_multi_offscreen_orbit [str] [str] [uint] [uint] [float]``
    Capture an N-view offscreen orbit: camera_id output_dir temporal_frame view_count yaw_span

``vset /mqrc/depth_of_field_scale [float]``
    Set depth of field scale

``vset /mqrc/exposure_bias [float]``
    Set exposure bias value

``vset /mqrc/exposure_method [str]``
    Set exposure method: histogram, basic, manual

``vset /mqrc/lumen_final_gather_lighting_update_speed [float]``
    Set Lumen final gather lighting update speed

``vset /mqrc/lumen_quality [float] [float]``
    Set Lumen quality settings: scene_quality, gather_quality

``vset /mqrc/motion_blur [float]``
    Set motion blur amount

``vset /mqrc/override_auto_exposure_max_brightness [bool]``
    Set auto-exposure maximum brightness override flag

``vset /mqrc/override_auto_exposure_min_brightness [bool]``
    Set auto-exposure minimum brightness override flag

``vset /mqrc/override_depth_of_field_scale [bool]``
    Set depth of field scale override flag

``vset /mqrc/override_exposure_bias [bool]``
    Set exposure bias override flag

``vset /mqrc/override_exposure_method [bool]``
    Set exposure method override flag

``vset /mqrc/override_lumen_final_gather_lighting_update_speed [bool]``
    Set override flag for Lumen final gather lighting update speed (true/false)

``vset /mqrc/override_lumen_final_gather_quality [bool]``
    Set Lumen final gather quality override flag

``vset /mqrc/override_lumen_scene_lighting_quality [bool]``
    Set Lumen scene lighting quality override flag

``vset /mqrc/override_motion_blur [bool]``
    Set motion blur amount override flag

``vset /mqrc/render_immediately [str]``
    Set render immediately mode: true, false

``vset /mqrc/reset_multi_offscreen_state [str]``
    Reset persistent multi-offscreen temporal state for a camera

``vset /mqrc/screen_percentage [float]``
    Set screen percentage (1.0 = 100%, 1.5 = 150% supersampling)

``vset /mqrc/screen_percentage_method [str]``
    Set primary screen percentage method: spatial, temporal, raw

Main-viewport rendering
-----------------------

``vget /mvrc/use_sync_capture``
    Get MainViewportRenderComponent sync capture mode status (0 or 1)

``vset /mvrc/use_sync_capture [uint]``
    Set MainViewportRenderComponent sync capture mode (0=disabled, 1=enabled)

Object
------

``vget /object/[str]/affect_distance_field_lighting``
    Get whether all primitive components of the object affect distance field lighting

``vget /object/[str]/cast_shadow``
    Get whether all primitive components of the object cast shadow

``vget /object/[str]/class_metadata``
    Get semantic class metadata for an object as JSON

``vget /object/[str]/materials_metadata``
    Get semantic materials metadata for an object as JSON

``vget /object/[str]/mesh_vertices``
    Get object mesh vertices as JSON

``vget /object/[str]/metadata``
    Get semantic metadata for an object as JSON

``vget /object/[str]/mujoco_go1_policy_obs``
    Get the 48D Go1 policy observation as JSON

``vget /object/[str]/mujoco_quadruped_pose_comparison``
    Get side-by-side UE and MuJoCo pose comparison for a quadruped actor

``vget /object/[str]/tickable_when_paused``
    Get whether the actor ticks while paused

``vset /object/[str]/affect_distance_field_lighting [str]``
    Set whether all primitive components of the object affect distance field lighting [true|false|1|0]

``vset /object/[str]/cast_shadow [str]``
    Set whether all primitive components of the object cast shadow [true|false|1|0]

``vset /object/[str]/hair_airdrag [float]``
    Set hair air drag for an actor with Groom component [0-1]

``vset /object/[str]/hair_gravity [float] [float] [float]``
    Set hair gravity for an actor with Groom component [x, y, z]

``vset /object/[str]/mujoco_freefall/start``
    Start MuJoCo freefall for a spawned cube and return the trajectory log path

``vset /object/[str]/mujoco_go1_policy_action [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float]``
    Set a normalized 12D Go1 policy action in [-1, 1] for the active MuJoCo Go1 component

``vset /object/[str]/mujoco_go1_policy_command [float] [float] [float]``
    Set the 3D Go1 policy command vector [vx, vy, yaw_rate] used by the policy observation

``vset /object/[str]/mujoco_humanoid_freefall/start``
    Start MuJoCo humanoid freefall for a skeletal actor and return the trajectory log path

``vset /object/[str]/mujoco_humanoid_pose_preview/start``
    Start MuJoCo humanoid pose preview for a skeletal actor and return the trajectory log path

``vset /object/[str]/mujoco_quadruped_freefall/start``
    Start MuJoCo quadruped freefall for a skeletal actor and return the trajectory log path

``vset /object/[str]/mujoco_quadruped_freefall/start [str]``
    Start MuJoCo quadruped freefall for a skeletal actor with an optional model spec and return the trajectory log path

``vset /object/[str]/mujoco_quadruped_pose_preview/start``
    Start MuJoCo quadruped pose preview for a skeletal actor and return the trajectory log path

``vset /object/[str]/mujoco_quadruped_pose_preview/start [str]``
    Start MuJoCo quadruped pose preview for a skeletal actor with an optional model spec and return the trajectory log path

``vset /object/[str]/reset_hair_simulation``
    Reset hair simulation for an actor with Groom component

``vset /object/[str]/settle_to_ground``
    Settle an existing object to the ground using bounds mode and large trace defaults

``vset /object/[str]/settle_to_ground [str]``
    Settle an existing object to the ground using mode: simple|bounds|sweep

``vset /object/[str]/settle_to_ground [str] [float] [float]``
    Settle an existing object to the ground with mode, trace_start_offset, and trace_length

``vset /object/[str]/settle_to_ground [str] [float] [float] [float]``
    Settle an existing object to the ground with mode, trace_start_offset, trace_length, and height_offset

``vset /object/[str]/tickable_when_paused [str]``
    Set whether the actor ticks while paused [true|false|1|0]

Object collection
-----------------

``vget /objects [str]``
    Get objects by name (case-insensitive search)

``vget /objects/scan_assets``
    Scan spawnable assets in /Game/ path

``vget /objects/scan_assets [str]``
    Scan spawnable assets in given path

``vset /objects/spawn_cube_wo_annotation``
    Spawn a box in the scene without annotation (legacy behavior).

``vset /objects/spawn_cube_wo_annotation [float] [float] [float]``
    Spawn a box in the scene at [x, y, z] without annotation (legacy behavior).

``vset /objects/spawn_cube_wo_annotation [str]``
    Spawn a named box in the scene without annotation (legacy behavior).

``vset /objects/spawn_cube_wo_annotation [str] [float] [float] [float]``
    Spawn a named box in the scene at [x, y, z] without annotation (legacy behavior).

``vset /objects/spawn_from_path [str]``
    Spawn an object from asset path (e.g., /Game/Folder/BP_Actor.BP_Actor). Auto-annotates by default.

``vset /objects/spawn_from_path [str] [float] [float] [float]``
    Spawn an object from asset path at [x, y, z]. Auto-annotates by default.

``vset /objects/spawn_from_path [str] [str]``
    Spawn an object from asset path with custom name. Auto-annotates by default.

``vset /objects/spawn_from_path [str] [str] [float] [float] [float]``
    Spawn a named object from asset path at [x, y, z]. Auto-annotates by default.

``vset /objects/spawn_from_path_wo_annotation [str]``
    Spawn an object from asset path without annotation (legacy behavior)

``vset /objects/spawn_from_path_wo_annotation [str] [float] [float] [float]``
    Spawn an object from asset path at [x, y, z] without annotation (legacy behavior)

``vset /objects/spawn_from_path_wo_annotation [str] [str]``
    Spawn an object from asset path with custom name without annotation (legacy behavior)

``vset /objects/spawn_from_path_wo_annotation [str] [str] [float] [float] [float]``
    Spawn a named object from asset path at [x, y, z] without annotation (legacy behavior)

``vset /objects/spawn_wo_annotation [str]``
    Spawn an object using UClassName first, then fall back to asset path if needed, without annotation (legacy behavior).

``vset /objects/spawn_wo_annotation [str] [float] [float] [float]``
    Spawn an object at [x, y, z] using UClassName first, then fall back to asset path if needed, without annotation (legacy behavior).

``vset /objects/spawn_wo_annotation [str] [str]``
    Spawn a named object using UClassName first, then fall back to asset path if needed, without annotation (legacy behavior).

``vset /objects/spawn_wo_annotation [str] [str] [float] [float] [float]``
    Spawn a named object at [x, y, z] using UClassName first, then fall back to asset path if needed, without annotation (legacy behavior).

PAK
---

``vget /pak/assets [str]``
    Get all assets in a package path. Args: [PackagePath]

``vget /pak/assets_in_pak [str]``
    Get asset package paths from a pak file. Args: [PakFilePath|PakIndex]

``vget /pak/files [str]``
    Get all files recorded in a pak file index. Args: [PakFilePath|PakIndex]

``vget /pak/ismounted [str]``
    Check if a pak file is mounted. Args: [PakFilePath|PakIndex]

``vget /pak/load [str]``
    Load an asset from pak. Args: [AssetPath]

``vget /pak/mounted``
    Get list of all mounted pak files with logical indices

``vget /pak/registered_paths [str]``
    Get all registered package paths from a pak file. Args: [PakFilePath|PakIndex]

``vget /pak/registered_status [str]``
    Get registered package paths and status from a pak file. Args: [PakFilePath|PakIndex]

``vset /pak/mount [str] [uint]``
    Mount a pak file at runtime. Args: [PakFilePath] [PakOrder=0]

``vset /pak/register [str] [str]``
    Register pak assets. Args: [PackagePath] [Category]

``vset /pak/scan [str] [uint]``
    Scan assets in mounted pak. Args: [MountPoint] [bForceRescan=1]

``vset /pak/unmount [str]``
    Unmount a pak file. Args: [PakFilePath|PakIndex]

Pawn
----

``vget /pawn/location``
    Get player pawn location [x, y, z]

``vget /pawn/rotation``
    Get player pawn rotation [pitch, yaw, roll]

``vset /pawn/location [float] [float] [float]``
    Set player pawn location [x, y, z]

``vset /pawn/rotation [float] [float] [float]``
    Set player pawn rotation [pitch, yaw, roll]

Reachable-area visualization
----------------------------

``vset /reachablearea/clear``
    Clear persistent reachable area debug draw

``vset /reachablearea/show [float] [float] [float] [float]``
    Show reachable points in-world by radius

``vset /reachablearea/show [float] [float] [float] [float] [float]``
    Show reachable points in-world by radius

``vset /reachablearea/show [float] [float] [float] [float] [float] [float]``
    Show reachable points in-world by radius

Reachable points
----------------

``vget /reachablepoints``
    Get cached reachable points as JSON

``vget /reachablepoints [float]``
    Get cached reachable points as JSON

``vget /reachablepoints [float] [float]``
    Get cached reachable points as JSON

``vget /reachablepoints/count``
    Get the number of cached reachable points

``vget /reachablepoints/count [float]``
    Get the number of cached reachable points

``vget /reachablepoints/count [float] [float]``
    Get the number of cached reachable points

``vget /reachablepoints/inradius [float] [float] [float] [float]``
    Get cached reachable points in radius as JSON

``vget /reachablepoints/inradius [float] [float] [float] [float] [float]``
    Get cached reachable points in radius as JSON

``vget /reachablepoints/inradius [float] [float] [float] [float] [float] [float]``
    Get cached reachable points in radius as JSON

``vget /reachablepoints/status``
    Get reachable point cache status

``vset /reachablepoints/invalidate``
    Invalidate cached reachable points

``vset /reachablepoints/invalidate [str]``
    Invalidate cached reachable points

``vset /reachablepoints/refresh``
    Force rebuild of reachable point context and default cache

Safe points
-----------

``vget /safepoint/config_path``
    Get safepoint config file path as JSON

``vset /safepoint/add [float] [float] [float]``
    Add safe point to current scene

``vset /safepoint/add [str] [float] [float] [float]``
    Add safe point to specified scene

``vset /safepoint/cycle``
    Cycle player pawn to next safe point

``vset /safepoint/preview_last``
    Preview the last safe point using current scene config

Safe-point collection
---------------------

``vget /safepoints``
    Get safe points of current scene as JSON

``vget /safepoints [str]``
    Get safe points of the specified scene as JSON

Scene
-----

``vget /scene/occupancy [str] [str]``
    Return or save a bool NPY occupancy grid: npy|filename profile.

``vget /scene/occupancy [str] [str] [float] [float] [float] [float] [uint]``
    Return or save occupancy: npy|filename profile origin_cm xyz yaw_degrees include_dynamic.

``vget /scene/occupancy [str] [str] [str]``
    Return or save occupancy: npy|filename profile method.

``vget /scene/occupancy [str] [str] [str] [float] [float] [float] [float] [uint]``
    Return or save occupancy: npy|filename profile method origin_cm xyz yaw_degrees include_dynamic.

``vget /scene/occupancy/spec [str]``
    Return LINGO occupancy profile metadata as JSON.

``vget /scene/occupancy/spec [str] [str]``
    Return LINGO occupancy profile and method metadata as JSON.

``vget /scene/occupancy_shared [str]``
    Return occupancy bool data through shared memory: profile.

``vget /scene/occupancy_shared [str] [float] [float] [float] [float] [uint]``
    Return shared occupancy: profile origin_cm xyz yaw_degrees include_dynamic.

``vget /scene/occupancy_shared [str] [str]``
    Return shared occupancy: profile method.

``vget /scene/occupancy_shared [str] [str] [float] [float] [float] [float] [uint]``
    Return shared occupancy: profile method origin_cm xyz yaw_degrees include_dynamic.

``vget /scene/perception``
    Return a compact native AI perception snapshot: map, agent pose, nearby objects, nav reachability, blockers and radial Visibility traces.

``vget /scene/perception [float] [uint] [uint]``
    Perception arguments: radius_cm max_objects max_rays.

``vget /scene/semantic_annotations``
    Get scene semantic annotations as JSON

Plugin
------

``vget /unrealcv/list_cmd``
    List all available commands and their help message

Vs
--

``vget /vs/dt``
    Compatibility alias for vget /bs/dt

World
-----

``vset /world/custom_time_dilation_except_pawn [float]``
    Set custom time dilation for all actors except the player pawn

``vset /world/pause_all_except_pawn``
    Pause all actors except the player pawn

``vset /world/resume_all``
    Resume all actors previously paused by /world/pause_all_except_pawn

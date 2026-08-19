"""High-level Python API for UnrealCV Plus servers.

This module contains commands implemented by UnrealCV Plus and keeps them
separate from the open-source UnrealCV API.
"""

import json
import warnings
from io import BytesIO

import numpy as np

from .api import UnrealCv_API

__all__ = ["UnrealCvPlusAPI"]


class UnrealCvPlusAPI(UnrealCv_API):
    """Open-source UnrealCV API plus the UnrealCV Plus command surface."""

    def get_scene_occupancy(self, profile='lingo_vis', origin_cm=None,
                            yaw_degrees=0.0, include_dynamic=False, method='bounds'):
        """Return a scene occupancy grid from an extended UnrealCV server."""
        cmd = self._build_scene_occupancy_command(
            'vget /scene/occupancy', profile, method, origin_cm,
            yaw_degrees, include_dynamic
        )
        payload = self._request_unrealcv_plus(cmd, 120)
        if not isinstance(payload, (bytes, bytearray, memoryview)):
            raise RuntimeError(f'Occupancy command did not return binary NPY data: {payload}')
        return np.load(BytesIO(bytes(payload)), allow_pickle=False)

    def save_scene_occupancy(self, path, profile='lingo_vis', origin_cm=None,
                             yaw_degrees=0.0, include_dynamic=False, method='bounds'):
        """Build and save a scene occupancy grid on an extended UnrealCV server."""
        cmd = self._build_scene_occupancy_command(
            f'vset /scene/occupancy/save {path}', profile, method, origin_cm,
            yaw_degrees, include_dynamic
        )
        return self._request_unrealcv_plus(cmd, 120)

    def get_scene_occupancy_spec(self, profile='lingo_vis', method='bounds'):
        """Return the selected scene occupancy grid contract as decoded JSON."""
        payload = self._request_unrealcv_plus(
            f'vget /scene/occupancy/spec {profile} {method}'
        )
        if not isinstance(payload, str):
            raise RuntimeError(f'Occupancy spec did not return JSON text: {payload}')
        return json.loads(payload)

    def get_scene_occupancy_region(
            self, min_m, max_m, voxel_size_m=0.1, method='bounds',
            origin_cm=None, yaw_degrees=0.0, include_dynamic=False):
        """Return occupancy for explicit x/y/z coverage bounds in meters.

        ``min_m`` and ``max_m`` are ``(x, y, z)`` bounds in grid-local meters.
        The voxel size is explicit so callers can request large regions without
        accidentally allocating an impractical 10 cm grid.
        """
        cmd = self._build_scene_occupancy_region_command(
            'vget /scene/occupancy_region npy', min_m, max_m, voxel_size_m,
            method, origin_cm, yaw_degrees, include_dynamic)
        payload = self._request_unrealcv_plus(cmd, 120)
        if not isinstance(payload, (bytes, bytearray, memoryview)):
            raise RuntimeError(f'Occupancy region command did not return binary NPY data: {payload}')
        return np.load(BytesIO(bytes(payload)), allow_pickle=False)

    def save_scene_occupancy_region(
            self, path, min_m, max_m, voxel_size_m=0.1, method='bounds',
            origin_cm=None, yaw_degrees=0.0, include_dynamic=False):
        """Save occupancy for explicit x/y/z coverage bounds in meters."""
        cmd = self._build_scene_occupancy_region_command(
            f'vget /scene/occupancy_region {path}', min_m, max_m,
            voxel_size_m, method, origin_cm, yaw_degrees, include_dynamic)
        return self._request_unrealcv_plus(cmd, 120)

    def get_scene_occupancy_region_spec(
            self, min_m, max_m, voxel_size_m=0.1, method='bounds'):
        """Return metadata for an explicit occupancy region."""
        cmd = self._build_scene_occupancy_region_command(
            'vget /scene/occupancy_region/spec', min_m, max_m,
            voxel_size_m, method)
        payload = self._request_unrealcv_plus(cmd)
        if not isinstance(payload, str):
            raise RuntimeError(f'Occupancy region spec did not return JSON text: {payload}')
        return json.loads(payload)

    @staticmethod
    def _build_scene_occupancy_command(prefix, profile, method, origin_cm,
                                       yaw_degrees, include_dynamic):
        if origin_cm is None and not yaw_degrees and not include_dynamic:
            return f'{prefix} {profile} {method}'
        if origin_cm is None:
            origin_cm = (0.0, 0.0, 0.0)
        if not hasattr(origin_cm, '__len__') or len(origin_cm) != 3:
            raise ValueError('origin_cm must contain exactly three values')
        try:
            origin = [float(value) for value in origin_cm]
            yaw = float(yaw_degrees)
        except (TypeError, ValueError) as exc:
            raise ValueError('origin_cm and yaw_degrees must be numeric') from exc
        dynamic = 1 if include_dynamic else 0
        return (
            f'{prefix} {profile} {method} '
            f'{origin[0]} {origin[1]} {origin[2]} {yaw} {dynamic}'
        )

    @staticmethod
    def _build_scene_occupancy_region_command(
            prefix, min_m, max_m, voxel_size_m, method='bounds',
            origin_cm=None, yaw_degrees=0.0, include_dynamic=False):
        if not hasattr(min_m, '__len__') or len(min_m) != 3:
            raise ValueError('min_m must contain exactly three values')
        if not hasattr(max_m, '__len__') or len(max_m) != 3:
            raise ValueError('max_m must contain exactly three values')
        try:
            minimum = [float(value) for value in min_m]
            maximum = [float(value) for value in max_m]
            voxel_size = float(voxel_size_m)
        except (TypeError, ValueError) as exc:
            raise ValueError('min_m, max_m, and voxel_size_m must be numeric') from exc
        if voxel_size <= 0 or any(upper <= lower for lower, upper in zip(minimum, maximum)):
            raise ValueError('max_m must be greater than min_m and voxel_size_m must be positive')
        if method not in ('bounds', 'mesh'):
            raise ValueError("method must be 'bounds' or 'mesh'")
        command = (
            f'{prefix} {method} {minimum[0]} {maximum[0]} '
            f'{minimum[1]} {maximum[1]} {minimum[2]} {maximum[2]} {voxel_size}'
        )
        if origin_cm is None and not yaw_degrees and not include_dynamic:
            return command
        if origin_cm is None or not hasattr(origin_cm, '__len__') or len(origin_cm) != 3:
            raise ValueError('origin_cm must contain exactly three values')
        try:
            origin = [float(value) for value in origin_cm]
            yaw = float(yaw_degrees)
        except (TypeError, ValueError) as exc:
            raise ValueError('origin_cm and yaw_degrees must be numeric') from exc
        return (
            f'{command} {origin[0]} {origin[1]} {origin[2]} {yaw} '
            f'{1 if include_dynamic else 0}'
        )
    

    def _warn_unrealcv_plus_if_unsupported(self):
        self.api_version.warn_unrealcv_plus_if_unsupported(stacklevel=2)

    def supports_command(self, command):
        """Return whether the connected server advertises ``command``.

        Returns ``None`` when the server is too old or otherwise unable to
        report its command table.
        """
        return self.api_version.supports_command(command)

    def is_unrealcv_plus(self):
        """Return whether the connected server is an UnrealCV Plus server."""
        return self.api_version.is_unrealcv_plus()

    def get_server_version(self):
        """Return the raw server version string, if available."""
        return self.api_version.get_server_version()

    def get_server_version_tuple(self):
        """Return the parsed ``(major, minor, patch)`` server version."""
        return self.api_version.get_server_version_tuple()

    def _request_unrealcv_plus(self, cmd, *args):
        self._warn_unrealcv_plus_if_unsupported()
        return self.client.request(cmd, *args)

    def spawn_free_camera(self, return_cmd=False):
        """
        Spawn a new free camera at the world origin (0, 0, 0).

        Args:
            return_cmd (bool): Whether to return the command string instead of executing it. Default is False.

        Returns:
            int: The camera ID of the newly spawned camera, or command string if return_cmd is True.
        """
        cmd = 'vset /captureactor/spawn_free_cam'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.isdigit():
            return int(res)
        raise ValueError(f"Error: {res}")

    def set_recording_time_dilation(self, dilation, return_cmd=False):
        cmd = f'vset /captureactor/time_dilation {dilation}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_camera_fast_capture(self, cam_id, return_cmd=False):
        """
        Get the fast capture mode status of a camera.
        """
        cmd = f'vget /camera/{cam_id}/use_fast_capture'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.isdigit():
            res = int(res)
            if res not in [0, 1]:
                raise ValueError(f"Invalid fast capture mode value: {res}")
            return res == 1
        raise ValueError(f"Error: {res}")

    def set_camera_fast_capture(self, cam_id, enabled, return_cmd=False):
        """
        Set the fast capture mode of a camera.
        """
        if isinstance(enabled, bool):
            enabled = 1 if enabled else 0
        if enabled not in [0, 1]:
            raise ValueError(f"Invalid fast capture mode value: {enabled}")
        cmd = f'vset /camera/{cam_id}/use_fast_capture {enabled}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd, -1)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def start_simple_recording(self, cam_id, output_folder, fps, duration_seconds, record_options=None, return_cmd=False):
        """
        Start simple recording without camera movement.
        """
        record_options = self._normalize_record_options(record_options)
        cmd = f'vset /captureactor/{cam_id}/record {output_folder} {fps} {duration_seconds}'
        if record_options:
            cmd += f' {record_options}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def is_recording(self, cam_id, return_cmd=False):
        """
        Check if a camera is currently recording.
        """
        cmd = f'vget /captureactor/{cam_id}/is_recording'
        if return_cmd:
            return cmd
        return self.decoder.string2bool(self._request_unrealcv_plus(cmd))

    def _parse_bool_response(self, res):
        if isinstance(res, str):
            if res in ['1', '0']:
                return res == '1'
            return self.decoder.string2bool(res)
        raise ValueError(f"Invalid boolean response type: {type(res)}")

    def _to_uint_flag(self, value):
        if isinstance(value, bool):
            return 1 if value else 0
        if value in [0, 1]:
            return value
        raise ValueError(f"Expected bool or 0/1, got: {value}")

    def _split_lines(self, res):
        if res is None:
            return []
        return [line for line in str(res).splitlines() if line.strip()]

    def _normalize_record_options(self, record_options):
        if record_options is None:
            return None
        if isinstance(record_options, str):
            return record_options
        if isinstance(record_options, (list, tuple)):
            return ','.join(str(option) for option in record_options)
        raise ValueError(f"Unsupported record_options type: {type(record_options)}")

    def get_camera_list_legacy(self, return_cmd=False):
        """
        Get legacy camera names returned by ``vget /cameras_legacy``.
        """
        cmd = 'vget /cameras_legacy'
        if return_cmd:
            return cmd
        return self._request_unrealcv_plus(cmd).split()

    def get_camera_list_cid(self, return_cmd=False):
        """
        Get stable UnrealCV Dev For UnrealZoo camera identifiers (CID format).
        """
        cmd = 'vget /cameras_CID'
        if return_cmd:
            return cmd
        return self._request_unrealcv_plus(cmd).split()

    def get_camera_id_map(self):
        """
        Pair legacy camera names with stable CID identifiers.
        """
        legacy_ids = self.get_camera_list_legacy()
        cid_ids = self.get_camera_list_cid()
        return [
            dict(index=index, legacy_id=legacy_id, cid=cid_id)
            for index, (legacy_id, cid_id) in enumerate(zip(legacy_ids, cid_ids))
        ]

    def annotate_object(self, actor_name, return_cmd=False):
        """
        Annotate a single actor by name.
        """
        cmd = f'vset /annotation/object/{actor_name}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def annotate_world(self, return_cmd=False):
        """
        Annotate the current world.
        """
        cmd = 'vset /annotation/world'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def clear_world_annotation(self, return_cmd=False):
        """
        Remove world annotation.
        """
        cmd = 'vset /annotation/world/clear'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def set_annotation_cache_enabled(self, enabled, return_cmd=False):
        """
        Enable or disable annotation component cache.
        """
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /annotation/cache/enable {enabled}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return self._parse_bool_response(res)

    def clear_annotation_cache(self, return_cmd=False):
        """
        Clear annotation component cache.
        """
        cmd = 'vset /annotation/cache/clear'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def mount_pak(self, pak_file_path, pak_order=0, return_cmd=False):
        """
        Mount a pak file at runtime.
        """
        cmd = f'vset /pak/mount {pak_file_path} {pak_order}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def unmount_pak(self, pak_file_path, return_cmd=False):
        """
        Unmount a pak file.
        """
        cmd = f'vset /pak/unmount {pak_file_path}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_mounted_paks(self, return_cmd=False):
        """
        Get all currently mounted pak files.
        """
        cmd = 'vget /pak/mounted'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res == 'No pak files mounted':
            return []
        return self._split_lines(res)

    def is_pak_mounted(self, pak_file_path, return_cmd=False):
        """
        Check whether a pak file is mounted.
        """
        cmd = f'vget /pak/ismounted {pak_file_path}'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self._request_unrealcv_plus(cmd))

    def get_pak_files(self, pak_file_path, return_cmd=False):
        """
        List raw file entries recorded in a pak file index.
        """
        cmd = f'vget /pak/files {pak_file_path}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return self._split_lines(res)

    def get_pak_assets_in_pak(self, pak_file_path, return_cmd=False):
        """
        List Unreal asset package paths discovered inside a pak file.
        """
        cmd = f'vget /pak/assets_in_pak {pak_file_path}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return self._split_lines(res)

    def scan_pak_assets(self, mount_point, force_rescan=True, return_cmd=False):
        """
        Scan assets from a mounted pak mount point.
        """
        force_rescan = self._to_uint_flag(force_rescan)
        cmd = f'vset /pak/scan {mount_point} {force_rescan}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def load_pak_asset(self, asset_path, return_cmd=False):
        """
        Load an asset from a mounted pak package path.
        """
        cmd = f'vget /pak/load {asset_path}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_pak_assets(self, package_path, return_cmd=False):
        """
        Get assets available under a package path.
        """
        cmd = f'vget /pak/assets {package_path}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return self._split_lines(res)

    def register_pak_assets(self, package_path, category, return_cmd=False):
        """
        Register pak assets into the UnrealCV asset pool.
        """
        cmd = f'vset /pak/register {package_path} {category}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def set_camera_panoramic_resolution(self, cam_id, cubemap_resolution, return_cmd=False):
        """
        Set the cubemap resolution used for panoramic capture.
        """
        cmd = f'vset /camera/{cam_id}/panoramic/resolution {cubemap_resolution}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def capture_panoramic(self, cam_id, path, width=None, height=None, return_cmd=False, timeout=5):
        """
        Capture a panoramic equirectangular image to file.
        """
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        cmd = f'vget /camera/{cam_id}/panoramic {path}'
        if width is not None and height is not None:
            cmd += f' {width} {height}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd, timeout)
        if timeout >= 0 and isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def capture_panoramic_normal(self, cam_id, path, width=None, height=None,
                                 return_cmd=False, timeout=5):
        """Capture a panoramic world-normal image to a server-side file."""
        return self._capture_panoramic_modality(
            cam_id, 'normal', path, width, height, return_cmd, timeout
        )

    def capture_panoramic_mask(self, cam_id, path, width=None, height=None,
                               return_cmd=False, timeout=5):
        """Capture a panoramic object-mask image to a server-side file."""
        return self._capture_panoramic_modality(
            cam_id, 'mask', path, width, height, return_cmd, timeout
        )

    def capture_panoramic_depth(self, cam_id, path, width=None, height=None,
                                return_cmd=False, timeout=5):
        """Capture a panoramic depth preview image to a server-side file."""
        return self._capture_panoramic_modality(
            cam_id, 'depth', path, width, height, return_cmd, timeout
        )

    def _capture_panoramic_modality(self, cam_id, modality, path, width, height,
                                    return_cmd, timeout):
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        cmd = f'vget /camera/{cam_id}/panoramic/{modality} {path}'
        if width is not None and height is not None:
            cmd += f' {width} {height}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd, timeout)
        if timeout >= 0 and isinstance(res, str) and res.startswith("error"):
            raise ValueError(res)
        return res

    def stop_recording(self, cam_id, return_cmd=False):
        """
        Stop an active recording for a camera.
        """
        cmd = f'vset /captureactor/{cam_id}/stop_record'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_use_movie_quality_rendering(self, return_cmd=False):
        cmd = 'vget /captureactor/use_movie_quality_rendering'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self._request_unrealcv_plus(cmd))

    def set_use_movie_quality_rendering(self, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/use_movie_quality_rendering {enabled}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_record_via_viewport(self, return_cmd=False):
        cmd = 'vget /captureactor/record_via_viewport'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self._request_unrealcv_plus(cmd))

    def set_record_via_viewport(self, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/record_via_viewport {enabled}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_warmup_frames(self, return_cmd=False):
        cmd = 'vget /captureactor/warmup_frames'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if not res.isdigit():
            raise ValueError(f"Error: {res}")
        return int(res)

    def set_warmup_frames(self, warmup_frames, return_cmd=False):
        cmd = f'vset /captureactor/warmup_frames {warmup_frames}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_paused_tick_interval(self, return_cmd=False):
        cmd = 'vget /captureactor/paused_tick_interval'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return float(res)

    def set_paused_tick_interval(self, tick_interval, return_cmd=False):
        cmd = f'vset /captureactor/paused_tick_interval {tick_interval}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_record_add_timestamp(self, cam_id, return_cmd=False):
        cmd = f'vget /captureactor/{cam_id}/add_timestamp'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self._request_unrealcv_plus(cmd))

    def set_record_add_timestamp(self, cam_id, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/{cam_id}/add_timestamp {enabled}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_recording_paused(self, cam_id, return_cmd=False):
        cmd = f'vget /captureactor/{cam_id}/paused'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self._request_unrealcv_plus(cmd))

    def set_recording_paused(self, cam_id, paused, return_cmd=False):
        paused = self._to_uint_flag(paused)
        cmd = f'vset /captureactor/{cam_id}/paused {paused}'
        if return_cmd:
            return cmd
        res = self._request_unrealcv_plus(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res



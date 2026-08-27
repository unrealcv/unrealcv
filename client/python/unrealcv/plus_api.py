"""High-level Python API for UnrealCV Plus servers.

This module contains commands implemented by UnrealCV Plus and keeps them
separate from the open-source UnrealCV API.
"""

import json
import warnings
from io import BytesIO

import numpy as np
import unrealcv

from .api import UnrealCv_API

__all__ = ["UnrealCvPlusAPI"]


class UnrealCvPlusAPI(UnrealCv_API):
    """Open-source UnrealCV API plus the UnrealCV Plus command surface."""

    def get_scene_occupancy(self, profile='lingo_vis', origin_cm=None,
                            yaw_degrees=0.0, include_dynamic=False, method='bounds'):
        """Return a scene occupancy grid from an extended UnrealCV server."""
        legacy_cmd = self._build_scene_occupancy_command(
            'vget /scene/occupancy', profile, method, origin_cm,
            yaw_degrees, include_dynamic
        )
        shared_cmd = self._build_scene_occupancy_command(
            'vget /scene/occupancy_shared', profile, method, origin_cm,
            yaw_degrees, include_dynamic
        )
        cmd = (
            unrealcv.SharedCommand(shared_cmd, 'npy')
            if self.api_version.supports_shared_command(shared_cmd)
            else legacy_cmd
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
        legacy_cmd = self._build_scene_occupancy_region_command(
            'vget /scene/occupancy_region npy', min_m, max_m, voxel_size_m,
            method, origin_cm, yaw_degrees, include_dynamic)
        shared_cmd = self._build_scene_occupancy_region_command(
            'vget /scene/occupancy_shared_region', min_m, max_m, voxel_size_m,
            method, origin_cm, yaw_degrees, include_dynamic)
        cmd = (
            unrealcv.SharedCommand(shared_cmd, 'npy')
            if self.api_version.supports_shared_command(shared_cmd)
            else legacy_cmd
        )
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

    @staticmethod
    def _validate_panoramic_size(width, height):
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        if width is not None:
            width = int(width)
            height = int(height)
            if width <= 0 or height <= 0:
                raise ValueError("width and height must be positive")
        return width, height

    def _get_shared_camera_image(self, shared_template, args, mode='png',
                                 return_cmd=False, timeout=5, inverse=False):
        shared_cmd = self._format_plus_template(shared_template, args)
        if not self.api_version.supports_shared_command(shared_cmd):
            raise RuntimeError(
                f"Server does not advertise the Shared Memory command: {shared_cmd}"
            )
        cmd = unrealcv.SharedCommand(shared_cmd, mode)
        if return_cmd:
            return cmd
        payload = self._request_unrealcv_plus(cmd, timeout)
        if mode == 'npy':
            return self.decoder.decode_depth(payload, inverse)
        return self.decoder.decode_img(payload, mode, inverse)

    def _get_shared_camera_array(self, shared_template, args,
                                 return_cmd=False, timeout=5):
        """Return an arbitrary Shared Memory NPY array without image reshaping."""
        shared_cmd = self._format_plus_template(shared_template, args)
        if not self.api_version.supports_shared_command(shared_cmd):
            raise RuntimeError(
                f"Server does not advertise the Shared Memory command: {shared_cmd}"
            )
        cmd = unrealcv.SharedCommand(shared_cmd, 'npy')
        if return_cmd:
            return cmd
        payload = self._request_unrealcv_plus(cmd, timeout)
        return np.load(BytesIO(payload), allow_pickle=False)

    def get_camera_panoramic_frame(self, cam_id, width=None, height=None,
                                   return_cmd=False, timeout=5):
        """Return panoramic RGB data, using the advertised Shared command."""
        width, height = self._validate_panoramic_size(width, height)
        template = (
            "vget /camera/[camera_id]/panoramic_shared"
            if width is None else
            "vget /camera/[camera_id]/panoramic_shared [uint] [uint]"
        )
        args = (cam_id,) if width is None else (cam_id, width, height)
        return self._get_shared_camera_image(template, args, return_cmd=return_cmd,
                                             timeout=timeout)

    def get_camera_panoramic_normal_frame(self, cam_id, width=None, height=None,
                                          return_cmd=False, timeout=5):
        """Return panoramic normal data, using the advertised Shared command."""
        width, height = self._validate_panoramic_size(width, height)
        template = (
            "vget /camera/[camera_id]/panoramic/normal_shared"
            if width is None else
            "vget /camera/[camera_id]/panoramic/normal_shared [uint] [uint]"
        )
        args = (cam_id,) if width is None else (cam_id, width, height)
        return self._get_shared_camera_image(template, args, return_cmd=return_cmd,
                                             timeout=timeout)

    def get_camera_panoramic_mask_frame(self, cam_id, width=None, height=None,
                                        return_cmd=False, timeout=5):
        """Return panoramic object-mask data, using Shared Memory."""
        width, height = self._validate_panoramic_size(width, height)
        template = (
            "vget /camera/[camera_id]/panoramic/mask_shared"
            if width is None else
            "vget /camera/[camera_id]/panoramic/mask_shared [uint] [uint]"
        )
        args = (cam_id,) if width is None else (cam_id, width, height)
        return self._get_shared_camera_image(template, args, return_cmd=return_cmd,
                                             timeout=timeout)

    def get_camera_panoramic_depth_frame(self, cam_id, width=None, height=None,
                                         return_cmd=False, timeout=5,
                                         inverse=False):
        """Return panoramic float32 depth data, using Shared Memory."""
        width, height = self._validate_panoramic_size(width, height)
        template = (
            "vget /camera/[camera_id]/panoramic/depth_shared"
            if width is None else
            "vget /camera/[camera_id]/panoramic/depth_shared [uint] [uint]"
        )
        args = (cam_id,) if width is None else (cam_id, width, height)
        return self._get_shared_camera_image(template, args, mode='npy',
                                             return_cmd=return_cmd,
                                             timeout=timeout, inverse=inverse)

    def get_camera_mqrc_lit_frame(self, cam_id, return_cmd=False, timeout=5):
        """Return MQRC lit BGRA data through the automatic Shared path."""
        return self._get_shared_camera_image(
            "vget /camera/[camera_id]/mqrc/lit_shared", (cam_id,),
            return_cmd=return_cmd, timeout=timeout)

    def get_camera_lidar_frame(self, cam_id, return_cmd=False, timeout=5):
        """Return the LiDAR XYZI point cloud as an ``N x 4`` NumPy array."""
        return self._get_shared_camera_array(
            "vget /camera/[camera_id]/lidar_shared", (cam_id,),
            return_cmd=return_cmd, timeout=timeout)

    def get_camera_mqrc_panoramic_frame(self, cam_id, width, height,
                                        face_resolution=None, return_cmd=False,
                                        timeout=5):
        """Return MQRC panoramic data with explicit capture dimensions."""
        width, height = self._validate_panoramic_size(width, height)
        if face_resolution is None:
            template = "vget /camera/[camera_id]/mqrc/panoramic_shared [uint] [uint]"
            args = (cam_id, width, height)
        else:
            face_resolution = int(face_resolution)
            if face_resolution <= 0:
                raise ValueError("face_resolution must be positive")
            template = (
                "vget /camera/[camera_id]/mqrc/panoramic_shared "
                "[uint] [uint] [uint]"
            )
            args = (cam_id, width, height, face_resolution)
        return self._get_shared_camera_image(template, args, return_cmd=return_cmd,
                                             timeout=timeout)

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




    @staticmethod
    def _format_plus_template(template, args):
        values = list(args)
        index = 0
        def replace(match):
            nonlocal index
            if index >= len(values):
                raise ValueError(f"Missing argument for {match.group(0)} in {template!r}")
            value = values[index]
            index += 1
            return str(value)
        import re
        command = re.sub(r"\[[^\[\]]+\]", replace, template)
        if index != len(values):
            raise ValueError(f"Too many arguments for {template!r}: {len(values)}")
        return command

    def _request_plus_template(self, template, args, return_cmd=False, timeout=5):
        command = self._format_plus_template(template, args)
        if return_cmd:
            return command
        return self._request_unrealcv_plus(command, timeout)

    def get_agent_nav_status(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /agent/[str]/nav/status``."""
        return self._request_plus_template("vget /agent/[str]/nav/status", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_agent_nav_goto(self, str_arg1, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /agent/[str]/nav/goto [float] [float] [float]``."""
        return self._request_plus_template("vset /agent/[str]/nav/goto [float] [float] [float]", (str_arg1, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_agent_nav_start(self, str_arg1, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /agent/[str]/nav/start [float]``."""
        return self._request_plus_template("vset /agent/[str]/nav/start [float]", (str_arg1, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_agent_nav_stop(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /agent/[str]/nav/stop``."""
        return self._request_plus_template("vset /agent/[str]/nav/stop", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_animation_soma_glb_status(self, actor_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /animation/soma_glb/status [str]``."""
        return self._request_plus_template("vget /animation/soma_glb/status [str]", (actor_name,), return_cmd=return_cmd, timeout=timeout)

    def set_animation_soma_glb_apply(self, actor_name, value, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /animation/soma_glb/apply [str] [Anything]``."""
        return self._request_plus_template("vset /animation/soma_glb/apply [str] [Anything]", (actor_name, value), return_cmd=return_cmd, timeout=timeout)

    def set_animation_soma_glb_stop(self, actor_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /animation/soma_glb/stop [str]``."""
        return self._request_plus_template("vset /animation/soma_glb/stop [str]", (actor_name,), return_cmd=return_cmd, timeout=timeout)

    def set_annotation_cache_clear(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /annotation/cache/clear``."""
        return self._request_plus_template("vset /annotation/cache/clear", (), return_cmd=return_cmd, timeout=timeout)

    def set_annotation_cache_enable(self, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /annotation/cache/enable [uint]``."""
        return self._request_plus_template("vset /annotation/cache/enable [uint]", (uint_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_annotation_object(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /annotation/object/[str]``."""
        return self._request_plus_template("vset /annotation/object/[str]", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_annotation_world(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /annotation/world``."""
        return self._request_plus_template("vset /annotation/world", (), return_cmd=return_cmd, timeout=timeout)

    def set_annotation_world_clear(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /annotation/world/clear``."""
        return self._request_plus_template("vset /annotation/world/clear", (), return_cmd=return_cmd, timeout=timeout)

    def get_camera_depth_exp(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/depth/exp``."""
        return self._request_plus_template("vget /camera/[camera_id]/depth/exp", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_depth_max_distance(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/depth/max_distance``."""
        return self._request_plus_template("vget /camera/[camera_id]/depth/max_distance", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_depth_min_distance(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/depth/min_distance``."""
        return self._request_plus_template("vget /camera/[camera_id]/depth/min_distance", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_depth_use_exp(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/depth/use_exp``."""
        return self._request_plus_template("vget /camera/[camera_id]/depth/use_exp", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_dual_depth(self, cam_id, foreground_path, background_path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/dual_depth [str] [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/dual_depth [str] [str]", (cam_id, foreground_path, background_path), return_cmd=return_cmd, timeout=timeout)

    def get_camera_id(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/id``."""
        return self._request_plus_template("vget /camera/[camera_id]/id", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mqrc_lit(self, cam_id, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/mqrc/lit [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/mqrc/lit [str]", (cam_id, str_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mvrc_enabled(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/mvrc/enabled``."""
        return self._request_plus_template("vget /camera/[camera_id]/mvrc/enabled", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mvrc_lit(self, cam_id, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/mvrc/lit [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/mvrc/lit [str]", (cam_id, str_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_camera_oneobjlit(self, cam_id, str_arg1, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/oneobjlit [str] [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/oneobjlit [str] [str]", (cam_id, str_arg1, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_camera_oneobjlit_legacy(self, cam_id, str_arg1, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/oneobjlit_legacy [str] [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/oneobjlit_legacy [str] [str]", (cam_id, str_arg1, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_camera_oneobjmask(self, cam_id, str_arg1, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/oneobjmask [str] [str]``."""
        return self._request_plus_template("vget /camera/[camera_id]/oneobjmask [str] [str]", (cam_id, str_arg1, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic(self, cam_id, path, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic [str] [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id, path) if width is None else (cam_id, path, width, height)
        template = "vget /camera/[camera_id]/panoramic [str]" if width is None else "vget /camera/[camera_id]/panoramic [str] [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_depth(self, cam_id, path, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/depth [str] [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id, path) if width is None else (cam_id, path, width, height)
        template = "vget /camera/[camera_id]/panoramic/depth [str]" if width is None else "vget /camera/[camera_id]/panoramic/depth [str] [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_depth_shared(self, cam_id, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/depth_shared [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id,) if width is None else (cam_id, width, height)
        template = "vget /camera/[camera_id]/panoramic/depth_shared" if width is None else "vget /camera/[camera_id]/panoramic/depth_shared [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_depth_shared_default(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the default-size panoramic depth shared-memory command."""
        return self._request_plus_template("vget /camera/[camera_id]/panoramic/depth_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_depth_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/depth_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/depth_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_lidar_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/lidar_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/lidar_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_lit_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/lit_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/lit_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mqrc_lit_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/mqrc/lit_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/mqrc/lit_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mqrc_panoramic_shared(self, cam_id, uint_arg1, uint_arg2, return_cmd=False, timeout=5):
        """Wrap the sized UnrealCV Plus MQRC panoramic shared-memory command."""
        return self._request_plus_template("vget /camera/[camera_id]/mqrc/panoramic_shared [uint] [uint]", (cam_id, uint_arg1, uint_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_camera_mqrc_panoramic_shared_with_face_resolution(self, cam_id, uint_arg1, uint_arg2, uint_arg3, return_cmd=False, timeout=5):
        """Wrap the MQRC panoramic shared-memory command with face resolution."""
        return self._request_plus_template("vget /camera/[camera_id]/mqrc/panoramic_shared [uint] [uint] [uint]", (cam_id, uint_arg1, uint_arg2, uint_arg3), return_cmd=return_cmd, timeout=timeout)

    def get_camera_normal_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/normal_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/normal_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_object_mask_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/object_mask_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/object_mask_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_seg_shared(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/seg_shared``."""
        return self._request_plus_template("vget /camera/[camera_id]/seg_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_mask(self, cam_id, path, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/mask [str] [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id, path) if width is None else (cam_id, path, width, height)
        template = "vget /camera/[camera_id]/panoramic/mask [str]" if width is None else "vget /camera/[camera_id]/panoramic/mask [str] [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_mask_shared(self, cam_id, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/mask_shared [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id,) if width is None else (cam_id, width, height)
        template = "vget /camera/[camera_id]/panoramic/mask_shared" if width is None else "vget /camera/[camera_id]/panoramic/mask_shared [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_mask_shared_default(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the default-size panoramic mask shared-memory command."""
        return self._request_plus_template("vget /camera/[camera_id]/panoramic/mask_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_normal(self, cam_id, path, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/normal [str] [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id, path) if width is None else (cam_id, path, width, height)
        template = "vget /camera/[camera_id]/panoramic/normal [str]" if width is None else "vget /camera/[camera_id]/panoramic/normal [str] [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_normal_shared(self, cam_id, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic/normal_shared [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id,) if width is None else (cam_id, width, height)
        template = "vget /camera/[camera_id]/panoramic/normal_shared" if width is None else "vget /camera/[camera_id]/panoramic/normal_shared [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_normal_shared_default(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the default-size panoramic normal shared-memory command."""
        return self._request_plus_template("vget /camera/[camera_id]/panoramic/normal_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_shared(self, cam_id, width=None, height=None, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/panoramic_shared [uint] [uint]``."""
        if (width is None) != (height is None):
            raise ValueError("width and height must be provided together")
        args = (cam_id,) if width is None else (cam_id, width, height)
        template = "vget /camera/[camera_id]/panoramic_shared" if width is None else "vget /camera/[camera_id]/panoramic_shared [uint] [uint]"
        return self._request_plus_template(template, args, return_cmd=return_cmd, timeout=timeout)

    def get_camera_panoramic_shared_default(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the default-size panoramic shared-memory command."""
        return self._request_plus_template("vget /camera/[camera_id]/panoramic_shared", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_render_in_main_renderer(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/render_in_main_renderer``."""
        return self._request_plus_template("vget /camera/[camera_id]/render_in_main_renderer", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def get_camera_use_fast_capture(self, cam_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /camera/[camera_id]/use_fast_capture``."""
        return self._request_plus_template("vget /camera/[camera_id]/use_fast_capture", (cam_id,), return_cmd=return_cmd, timeout=timeout)

    def set_camera_depth_exp(self, cam_id, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/depth/exp [float]``."""
        return self._request_plus_template("vset /camera/[camera_id]/depth/exp [float]", (cam_id, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_camera_depth_max_distance(self, cam_id, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/depth/max_distance [float]``."""
        return self._request_plus_template("vset /camera/[camera_id]/depth/max_distance [float]", (cam_id, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_camera_depth_min_distance(self, cam_id, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/depth/min_distance [float]``."""
        return self._request_plus_template("vset /camera/[camera_id]/depth/min_distance [float]", (cam_id, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_camera_depth_use_exp(self, cam_id, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/depth/use_exp [uint]``."""
        return self._request_plus_template("vset /camera/[camera_id]/depth/use_exp [uint]", (cam_id, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_camera_lookat_object_auto(self, cam_id, object_name, float_arg1, float_arg2, float_arg3, float_arg4, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/lookat_object_auto [str] [float] [float] [float] [float]``."""
        return self._request_plus_template("vset /camera/[camera_id]/lookat_object_auto [str] [float] [float] [float] [float]", (cam_id, object_name, float_arg1, float_arg2, float_arg3, float_arg4), return_cmd=return_cmd, timeout=timeout)

    def set_camera_render_in_main_renderer(self, cam_id, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/render_in_main_renderer [uint]``."""
        return self._request_plus_template("vset /camera/[camera_id]/render_in_main_renderer [uint]", (cam_id, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_camera_use_fast_capture(self, cam_id, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /camera/[camera_id]/use_fast_capture [uint]``."""
        return self._request_plus_template("vset /camera/[camera_id]/use_fast_capture [uint]", (cam_id, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_cameras_ids(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /cameras/ids``."""
        return self._request_plus_template("vget /cameras/ids", (), return_cmd=return_cmd, timeout=timeout)

    def get_cameras_cid(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /cameras_CID``."""
        return self._request_plus_template("vget /cameras_CID", (), return_cmd=return_cmd, timeout=timeout)

    def get_cameras_legacy(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /cameras_legacy``."""
        return self._request_plus_template("vget /cameras_legacy", (), return_cmd=return_cmd, timeout=timeout)

    def set_editor_start_standalone_pie(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /editor/start_standalone_pie``."""
        return self._request_plus_template("vset /editor/start_standalone_pie", (), return_cmd=return_cmd, timeout=timeout)

    def get_light_directional_castdeepshadow(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /light/directional/castdeepshadow``."""
        return self._request_plus_template("vget /light/directional/castdeepshadow", (), return_cmd=return_cmd, timeout=timeout)

    def get_light_directional_intensity(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /light/directional/intensity``."""
        return self._request_plus_template("vget /light/directional/intensity", (), return_cmd=return_cmd, timeout=timeout)

    def get_light_skylight_intensity(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /light/skylight/intensity``."""
        return self._request_plus_template("vget /light/skylight/intensity", (), return_cmd=return_cmd, timeout=timeout)

    def set_light_directional_castdeepshadow(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /light/directional/castdeepshadow [bool]``."""
        return self._request_plus_template("vset /light/directional/castdeepshadow [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_light_directional_intensity(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /light/directional/intensity [float]``."""
        return self._request_plus_template("vset /light/directional/intensity [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_light_skylight_intensity(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /light/skylight/intensity [float]``."""
        return self._request_plus_template("vset /light/skylight/intensity [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_llm_config(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /llm/config``."""
        return self._request_plus_template("vget /llm/config", (), return_cmd=return_cmd, timeout=timeout)

    def get_llm_request_result(self, request_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /llm/request/[str]/result``."""
        return self._request_plus_template("vget /llm/request/[str]/result", (request_id,), return_cmd=return_cmd, timeout=timeout)

    def get_llm_request_status(self, request_id, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /llm/request/[str]/status``."""
        return self._request_plus_template("vget /llm/request/[str]/status", (request_id,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_chat(self, value, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/chat [Anything]``."""
        return self._request_plus_template("vset /llm/chat [Anything]", (value,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_chat_json(self, value, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/chat_json [Anything]``."""
        return self._request_plus_template("vset /llm/chat_json [Anything]", (value,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_config_api_key(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/config/api_key [str]``."""
        return self._request_plus_template("vset /llm/config/api_key [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_config_base_url(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/config/base_url [str]``."""
        return self._request_plus_template("vset /llm/config/base_url [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_config_model(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/config/model [str]``."""
        return self._request_plus_template("vset /llm/config/model [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_llm_config_wire_api(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /llm/config/wire_api [str]``."""
        return self._request_plus_template("vset /llm/config/wire_api [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_metahuman_all_paths(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /metahuman/all_paths``."""
        return self._request_plus_template("vget /metahuman/all_paths", (), return_cmd=return_cmd, timeout=timeout)

    def get_metahuman_cache_path(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /metahuman/cache_path``."""
        return self._request_plus_template("vget /metahuman/cache_path", (), return_cmd=return_cmd, timeout=timeout)

    def get_metahuman_filter_batch(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /metahuman/filter_batch``."""
        return self._request_plus_template("vget /metahuman/filter_batch", (), return_cmd=return_cmd, timeout=timeout)

    def get_metahuman_head_aim_location(self, str_arg1, str_arg2, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /metahuman/head_aim_location [str] [str] [float]``."""
        return self._request_plus_template("vget /metahuman/head_aim_location [str] [str] [float]", (str_arg1, str_arg2, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_metahuman_parametric_rig_status(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /metahuman/parametric/rig_status [str]``."""
        return self._request_plus_template("vget /metahuman/parametric/rig_status [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_metahuman_parametric_assemble(self, str_arg1, str_arg2, str_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /metahuman/parametric/assemble [str] [str] [str]``."""
        return self._request_plus_template("vset /metahuman/parametric/assemble [str] [str] [str]", (str_arg1, str_arg2, str_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_metahuman_parametric_body(self, str_arg1, str_arg2, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /metahuman/parametric/body [str] [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /metahuman/parametric/body [str] [str] [float] [float] [float]", (str_arg1, str_arg2, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_metahuman_parametric_create(self, str_arg1, str_arg2, str_arg3, float_arg1, float_arg2, float_arg3, str_arg4, str_arg5, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /metahuman/parametric/create [str] [str] [str] [float] [float] [float] [str] [str]``."""
        return self._request_plus_template("vset /metahuman/parametric/create [str] [str] [str] [float] [float] [float] [str] [str]", (str_arg1, str_arg2, str_arg3, float_arg1, float_arg2, float_arg3, str_arg4, str_arg5), return_cmd=return_cmd, timeout=timeout)

    def set_metahuman_parametric_wardrobe(self, str_arg1, str_arg2, str_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /metahuman/parametric/wardrobe [str] [str] [str]``."""
        return self._request_plus_template("vset /metahuman/parametric/wardrobe [str] [str] [str]", (str_arg1, str_arg2, str_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_metahuman_update_cache(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /metahuman/update_cache``."""
        return self._request_plus_template("vset /metahuman/update_cache", (), return_cmd=return_cmd, timeout=timeout)

    def get_status(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget status``."""
        return self._request_plus_template("vget status", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_antialiasing(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/antialiasing``."""
        return self._request_plus_template("vget /mqrc/antialiasing", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_auto_exposure_max_brightness(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/auto_exposure_max_brightness``."""
        return self._request_plus_template("vget /mqrc/auto_exposure_max_brightness", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_auto_exposure_min_brightness(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/auto_exposure_min_brightness``."""
        return self._request_plus_template("vget /mqrc/auto_exposure_min_brightness", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_depth_of_field_scale(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/depth_of_field_scale``."""
        return self._request_plus_template("vget /mqrc/depth_of_field_scale", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_exposure_bias(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/exposure_bias``."""
        return self._request_plus_template("vget /mqrc/exposure_bias", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_exposure_method(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/exposure_method``."""
        return self._request_plus_template("vget /mqrc/exposure_method", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_lumen_final_gather_lighting_update_speed(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/lumen_final_gather_lighting_update_speed``."""
        return self._request_plus_template("vget /mqrc/lumen_final_gather_lighting_update_speed", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_lumen_quality(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/lumen_quality``."""
        return self._request_plus_template("vget /mqrc/lumen_quality", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_motion_blur(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/motion_blur``."""
        return self._request_plus_template("vget /mqrc/motion_blur", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_auto_exposure_max_brightness(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_auto_exposure_max_brightness``."""
        return self._request_plus_template("vget /mqrc/override_auto_exposure_max_brightness", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_auto_exposure_min_brightness(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_auto_exposure_min_brightness``."""
        return self._request_plus_template("vget /mqrc/override_auto_exposure_min_brightness", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_depth_of_field_scale(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_depth_of_field_scale``."""
        return self._request_plus_template("vget /mqrc/override_depth_of_field_scale", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_exposure_bias(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_exposure_bias``."""
        return self._request_plus_template("vget /mqrc/override_exposure_bias", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_exposure_method(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_exposure_method``."""
        return self._request_plus_template("vget /mqrc/override_exposure_method", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_lumen_final_gather_lighting_update_speed(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_lumen_final_gather_lighting_update_speed``."""
        return self._request_plus_template("vget /mqrc/override_lumen_final_gather_lighting_update_speed", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_lumen_final_gather_quality(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_lumen_final_gather_quality``."""
        return self._request_plus_template("vget /mqrc/override_lumen_final_gather_quality", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_lumen_scene_lighting_quality(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_lumen_scene_lighting_quality``."""
        return self._request_plus_template("vget /mqrc/override_lumen_scene_lighting_quality", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_override_motion_blur(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/override_motion_blur``."""
        return self._request_plus_template("vget /mqrc/override_motion_blur", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_render_immediately(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/render_immediately``."""
        return self._request_plus_template("vget /mqrc/render_immediately", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_screen_percentage(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/screen_percentage``."""
        return self._request_plus_template("vget /mqrc/screen_percentage", (), return_cmd=return_cmd, timeout=timeout)

    def get_mqrc_screen_percentage_method(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mqrc/screen_percentage_method``."""
        return self._request_plus_template("vget /mqrc/screen_percentage_method", (), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_antialiasing(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/antialiasing [str]``."""
        return self._request_plus_template("vset /mqrc/antialiasing [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_auto_exposure_max_brightness(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/auto_exposure_max_brightness [float]``."""
        return self._request_plus_template("vset /mqrc/auto_exposure_max_brightness [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_auto_exposure_min_brightness(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/auto_exposure_min_brightness [float]``."""
        return self._request_plus_template("vset /mqrc/auto_exposure_min_brightness [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_capture_multi_offscreen_orbit(self, path, str_arg2, uint_arg1, uint_arg2, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/capture_multi_offscreen_orbit [str] [str] [uint] [uint] [float]``."""
        return self._request_plus_template("vset /mqrc/capture_multi_offscreen_orbit [str] [str] [uint] [uint] [float]", (path, str_arg2, uint_arg1, uint_arg2, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_depth_of_field_scale(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/depth_of_field_scale [float]``."""
        return self._request_plus_template("vset /mqrc/depth_of_field_scale [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_exposure_bias(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/exposure_bias [float]``."""
        return self._request_plus_template("vset /mqrc/exposure_bias [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_exposure_method(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/exposure_method [str]``."""
        return self._request_plus_template("vset /mqrc/exposure_method [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_lumen_final_gather_lighting_update_speed(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/lumen_final_gather_lighting_update_speed [float]``."""
        return self._request_plus_template("vset /mqrc/lumen_final_gather_lighting_update_speed [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_lumen_quality(self, float_arg1, float_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/lumen_quality [float] [float]``."""
        return self._request_plus_template("vset /mqrc/lumen_quality [float] [float]", (float_arg1, float_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_motion_blur(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/motion_blur [float]``."""
        return self._request_plus_template("vset /mqrc/motion_blur [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_auto_exposure_max_brightness(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_auto_exposure_max_brightness [bool]``."""
        return self._request_plus_template("vset /mqrc/override_auto_exposure_max_brightness [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_auto_exposure_min_brightness(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_auto_exposure_min_brightness [bool]``."""
        return self._request_plus_template("vset /mqrc/override_auto_exposure_min_brightness [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_depth_of_field_scale(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_depth_of_field_scale [bool]``."""
        return self._request_plus_template("vset /mqrc/override_depth_of_field_scale [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_exposure_bias(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_exposure_bias [bool]``."""
        return self._request_plus_template("vset /mqrc/override_exposure_bias [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_exposure_method(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_exposure_method [bool]``."""
        return self._request_plus_template("vset /mqrc/override_exposure_method [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_lumen_final_gather_lighting_update_speed(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_lumen_final_gather_lighting_update_speed [bool]``."""
        return self._request_plus_template("vset /mqrc/override_lumen_final_gather_lighting_update_speed [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_lumen_final_gather_quality(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_lumen_final_gather_quality [bool]``."""
        return self._request_plus_template("vset /mqrc/override_lumen_final_gather_quality [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_lumen_scene_lighting_quality(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_lumen_scene_lighting_quality [bool]``."""
        return self._request_plus_template("vset /mqrc/override_lumen_scene_lighting_quality [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_override_motion_blur(self, bool_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/override_motion_blur [bool]``."""
        return self._request_plus_template("vset /mqrc/override_motion_blur [bool]", (bool_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_render_immediately(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/render_immediately [str]``."""
        return self._request_plus_template("vset /mqrc/render_immediately [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_reset_multi_offscreen_state(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/reset_multi_offscreen_state [str]``."""
        return self._request_plus_template("vset /mqrc/reset_multi_offscreen_state [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_screen_percentage(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/screen_percentage [float]``."""
        return self._request_plus_template("vset /mqrc/screen_percentage [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_mqrc_screen_percentage_method(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mqrc/screen_percentage_method [str]``."""
        return self._request_plus_template("vset /mqrc/screen_percentage_method [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_mvrc_use_sync_capture(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /mvrc/use_sync_capture``."""
        return self._request_plus_template("vget /mvrc/use_sync_capture", (), return_cmd=return_cmd, timeout=timeout)

    def set_mvrc_use_sync_capture(self, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /mvrc/use_sync_capture [uint]``."""
        return self._request_plus_template("vset /mvrc/use_sync_capture [uint]", (uint_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_object_affect_distance_field_lighting(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/affect_distance_field_lighting``."""
        return self._request_plus_template("vget /object/[str]/affect_distance_field_lighting", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_cast_shadow(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/cast_shadow``."""
        return self._request_plus_template("vget /object/[str]/cast_shadow", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_class_metadata(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/class_metadata``."""
        return self._request_plus_template("vget /object/[str]/class_metadata", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_materials_metadata(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/materials_metadata``."""
        return self._request_plus_template("vget /object/[str]/materials_metadata", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_mesh_vertices(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/mesh_vertices``."""
        return self._request_plus_template("vget /object/[str]/mesh_vertices", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_metadata(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/metadata``."""
        return self._request_plus_template("vget /object/[str]/metadata", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_mujoco_go1_policy_obs(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/mujoco_go1_policy_obs``."""
        return self._request_plus_template("vget /object/[str]/mujoco_go1_policy_obs", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_mujoco_quadruped_pose_comparison(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/mujoco_quadruped_pose_comparison``."""
        return self._request_plus_template("vget /object/[str]/mujoco_quadruped_pose_comparison", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def get_object_tickable_when_paused(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /object/[str]/tickable_when_paused``."""
        return self._request_plus_template("vget /object/[str]/tickable_when_paused", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_object_affect_distance_field_lighting(self, object_name, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/affect_distance_field_lighting [str]``."""
        return self._request_plus_template("vset /object/[str]/affect_distance_field_lighting [str]", (object_name, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_object_cast_shadow(self, object_name, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/cast_shadow [str]``."""
        return self._request_plus_template("vset /object/[str]/cast_shadow [str]", (object_name, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_object_hair_airdrag(self, object_name, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/hair_airdrag [float]``."""
        return self._request_plus_template("vset /object/[str]/hair_airdrag [float]", (object_name, float_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_object_hair_gravity(self, object_name, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/hair_gravity [float] [float] [float]``."""
        return self._request_plus_template("vset /object/[str]/hair_gravity [float] [float] [float]", (object_name, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_freefall_start(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_freefall/start``."""
        return self._request_plus_template("vset /object/[str]/mujoco_freefall/start", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_go1_policy_action(self, object_name, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, float_arg12, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_go1_policy_action [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vset /object/[str]/mujoco_go1_policy_action [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float]", (object_name, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, float_arg12), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_go1_policy_command(self, object_name, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_go1_policy_command [float] [float] [float]``."""
        return self._request_plus_template("vset /object/[str]/mujoco_go1_policy_command [float] [float] [float]", (object_name, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_humanoid_freefall_start(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_humanoid_freefall/start``."""
        return self._request_plus_template("vset /object/[str]/mujoco_humanoid_freefall/start", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_humanoid_pose_preview_start(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_humanoid_pose_preview/start``."""
        return self._request_plus_template("vset /object/[str]/mujoco_humanoid_pose_preview/start", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_quadruped_freefall_start(self, object_name, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_quadruped_freefall/start [str]``."""
        return self._request_plus_template("vset /object/[str]/mujoco_quadruped_freefall/start [str]", (object_name, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_object_mujoco_quadruped_pose_preview_start(self, object_name, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/mujoco_quadruped_pose_preview/start [str]``."""
        return self._request_plus_template("vset /object/[str]/mujoco_quadruped_pose_preview/start [str]", (object_name, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_object_reset_hair_simulation(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/reset_hair_simulation``."""
        return self._request_plus_template("vset /object/[str]/reset_hair_simulation", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_object_settle_to_ground(self, object_name, str_arg2, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/settle_to_ground [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /object/[str]/settle_to_ground [str] [float] [float] [float]", (object_name, str_arg2, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_object_tickable_when_paused(self, object_name, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /object/[str]/tickable_when_paused [str]``."""
        return self._request_plus_template("vset /object/[str]/tickable_when_paused [str]", (object_name, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_objects(self, object_filter=None, return_cmd=False, timeout=5):
        """Return all objects, or wrap ``vget /objects [str]`` when a filter is supplied."""
        if object_filter is None:
            if return_cmd:
                return 'vget /objects'
            return super().get_objects()
        return self._request_plus_template("vget /objects [str]", (object_filter,), return_cmd=return_cmd, timeout=timeout)

    def get_objects_filtered(self, object_filter, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /objects [str]``."""
        return self._request_plus_template("vget /objects [str]", (object_filter,), return_cmd=return_cmd, timeout=timeout)

    def get_objects_scan_assets(self, object_name, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /objects/scan_assets [str]``."""
        return self._request_plus_template("vget /objects/scan_assets [str]", (object_name,), return_cmd=return_cmd, timeout=timeout)

    def set_objects_spawn_cube_wo_annotation(self, object_name, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /objects/spawn_cube_wo_annotation [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /objects/spawn_cube_wo_annotation [str] [float] [float] [float]", (object_name, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_objects_spawn_from_path(self, object_name, str_arg2, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /objects/spawn_from_path [str] [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /objects/spawn_from_path [str] [str] [float] [float] [float]", (object_name, str_arg2, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_objects_spawn_from_path_wo_annotation(self, object_name, str_arg2, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /objects/spawn_from_path_wo_annotation [str] [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /objects/spawn_from_path_wo_annotation [str] [str] [float] [float] [float]", (object_name, str_arg2, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_objects_spawn_wo_annotation(self, object_name, str_arg2, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /objects/spawn_wo_annotation [str] [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /objects/spawn_wo_annotation [str] [str] [float] [float] [float]", (object_name, str_arg2, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def get_pak_ismounted(self, path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pak/ismounted [str]``."""
        return self._request_plus_template("vget /pak/ismounted [str]", (path,), return_cmd=return_cmd, timeout=timeout)

    def get_pak_load(self, path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pak/load [str]``."""
        return self._request_plus_template("vget /pak/load [str]", (path,), return_cmd=return_cmd, timeout=timeout)

    def get_pak_mounted(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pak/mounted``."""
        return self._request_plus_template("vget /pak/mounted", (), return_cmd=return_cmd, timeout=timeout)

    def get_pak_registered_paths(self, path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pak/registered_paths [str]``."""
        return self._request_plus_template("vget /pak/registered_paths [str]", (path,), return_cmd=return_cmd, timeout=timeout)

    def get_pak_registered_status(self, path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pak/registered_status [str]``."""
        return self._request_plus_template("vget /pak/registered_status [str]", (path,), return_cmd=return_cmd, timeout=timeout)

    def set_pak_mount(self, path, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pak/mount [str] [uint]``."""
        return self._request_plus_template("vset /pak/mount [str] [uint]", (path, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_pak_register(self, path, str_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pak/register [str] [str]``."""
        return self._request_plus_template("vset /pak/register [str] [str]", (path, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def set_pak_scan(self, path, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pak/scan [str] [uint]``."""
        return self._request_plus_template("vset /pak/scan [str] [uint]", (path, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def set_pak_unmount(self, path, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pak/unmount [str]``."""
        return self._request_plus_template("vset /pak/unmount [str]", (path,), return_cmd=return_cmd, timeout=timeout)

    def get_pawn_location(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pawn/location``."""
        return self._request_plus_template("vget /pawn/location", (), return_cmd=return_cmd, timeout=timeout)

    def get_pawn_rotation(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /pawn/rotation``."""
        return self._request_plus_template("vget /pawn/rotation", (), return_cmd=return_cmd, timeout=timeout)

    def set_pawn_location(self, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pawn/location [float] [float] [float]``."""
        return self._request_plus_template("vset /pawn/location [float] [float] [float]", (float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_pawn_rotation(self, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /pawn/rotation [float] [float] [float]``."""
        return self._request_plus_template("vset /pawn/rotation [float] [float] [float]", (float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_reachablearea_clear(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /reachablearea/clear``."""
        return self._request_plus_template("vset /reachablearea/clear", (), return_cmd=return_cmd, timeout=timeout)

    def set_reachablearea_show(self, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /reachablearea/show [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vset /reachablearea/show [float] [float] [float] [float] [float] [float]", (float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6), return_cmd=return_cmd, timeout=timeout)

    def get_reachablepoints(self, float_arg1, float_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /reachablepoints [float] [float]``."""
        return self._request_plus_template("vget /reachablepoints [float] [float]", (float_arg1, float_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_reachablepoints_count(self, float_arg1, float_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /reachablepoints/count [float] [float]``."""
        return self._request_plus_template("vget /reachablepoints/count [float] [float]", (float_arg1, float_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_reachablepoints_inradius(self, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /reachablepoints/inradius [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vget /reachablepoints/inradius [float] [float] [float] [float] [float] [float]", (float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6), return_cmd=return_cmd, timeout=timeout)

    def get_reachablepoints_status(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /reachablepoints/status``."""
        return self._request_plus_template("vget /reachablepoints/status", (), return_cmd=return_cmd, timeout=timeout)

    def set_reachablepoints_invalidate(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /reachablepoints/invalidate [str]``."""
        return self._request_plus_template("vset /reachablepoints/invalidate [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_reachablepoints_refresh(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /reachablepoints/refresh``."""
        return self._request_plus_template("vset /reachablepoints/refresh", (), return_cmd=return_cmd, timeout=timeout)

    def get_safepoint_config_path(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /safepoint/config_path``."""
        return self._request_plus_template("vget /safepoint/config_path", (), return_cmd=return_cmd, timeout=timeout)

    def set_safepoint_add(self, str_arg1, float_arg1, float_arg2, float_arg3, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /safepoint/add [str] [float] [float] [float]``."""
        return self._request_plus_template("vset /safepoint/add [str] [float] [float] [float]", (str_arg1, float_arg1, float_arg2, float_arg3), return_cmd=return_cmd, timeout=timeout)

    def set_safepoint_cycle(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /safepoint/cycle``."""
        return self._request_plus_template("vset /safepoint/cycle", (), return_cmd=return_cmd, timeout=timeout)

    def set_safepoint_preview_last(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /safepoint/preview_last``."""
        return self._request_plus_template("vset /safepoint/preview_last", (), return_cmd=return_cmd, timeout=timeout)

    def get_safepoints(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /safepoints [str]``."""
        return self._request_plus_template("vget /safepoints [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared(self, str_arg1, str_arg2, float_arg1, float_arg2, float_arg3, float_arg4, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared [str] [str] [float] [float] [float] [float] [uint]``."""
        return self._request_plus_template("vget /scene/occupancy_shared [str] [str] [float] [float] [float] [float] [uint]", (str_arg1, str_arg2, float_arg1, float_arg2, float_arg3, float_arg4, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_profile(self, str_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared [str]``."""
        return self._request_plus_template("vget /scene/occupancy_shared [str]", (str_arg1,), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_profile_method(self, str_arg1, str_arg2, return_cmd=False, timeout=5):
        """Wrap occupancy shared memory with explicit profile and method."""
        return self._request_plus_template("vget /scene/occupancy_shared [str] [str]", (str_arg1, str_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_transform(self, str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared [str] [float] [float] [float] [float] [uint]``."""
        return self._request_plus_template("vget /scene/occupancy_shared [str] [float] [float] [float] [float] [uint]", (str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_region(self, str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared_region [str] [float] [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vget /scene/occupancy_shared_region [str] [float] [float] [float] [float] [float] [float] [float]", (str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_region_transform_profile(self, str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1, return_cmd=False, timeout=5):
        """Wrap occupancy shared memory for an explicit profile and transform."""
        return self._request_plus_template("vget /scene/occupancy_shared_region [str] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [uint]", (str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_region_default(self, str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_region [str] [float] [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vget /scene/occupancy_region [str] [float] [float] [float] [float] [float] [float] [float]", (str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_region_transform(self, str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_region [str] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [uint]``."""
        return self._request_plus_template("vget /scene/occupancy_region [str] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [uint]", (str_arg1, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_region_default(self, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared_region [float] [float] [float] [float] [float] [float] [float]``."""
        return self._request_plus_template("vget /scene/occupancy_shared_region [float] [float] [float] [float] [float] [float] [float]", (float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7), return_cmd=return_cmd, timeout=timeout)

    def get_scene_occupancy_shared_region_transform(self, float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/occupancy_shared_region [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [uint]``."""
        return self._request_plus_template("vget /scene/occupancy_shared_region [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [float] [uint]", (float_arg1, float_arg2, float_arg3, float_arg4, float_arg5, float_arg6, float_arg7, float_arg8, float_arg9, float_arg10, float_arg11, uint_arg1), return_cmd=return_cmd, timeout=timeout)

    def get_scene_perception(self, float_arg1, uint_arg1, uint_arg2, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/perception [float] [uint] [uint]``."""
        return self._request_plus_template("vget /scene/perception [float] [uint] [uint]", (float_arg1, uint_arg1, uint_arg2), return_cmd=return_cmd, timeout=timeout)

    def get_scene_semantic_annotations(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /scene/semantic_annotations``."""
        return self._request_plus_template("vget /scene/semantic_annotations", (), return_cmd=return_cmd, timeout=timeout)

    def get_unrealcv_list_cmd(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vget /unrealcv/list_cmd``."""
        return self._request_plus_template("vget /unrealcv/list_cmd", (), return_cmd=return_cmd, timeout=timeout)

    def set_world_custom_time_dilation_except_pawn(self, float_arg1, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /world/custom_time_dilation_except_pawn [float]``."""
        return self._request_plus_template("vset /world/custom_time_dilation_except_pawn [float]", (float_arg1,), return_cmd=return_cmd, timeout=timeout)

    def set_world_pause_all_except_pawn(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /world/pause_all_except_pawn``."""
        return self._request_plus_template("vset /world/pause_all_except_pawn", (), return_cmd=return_cmd, timeout=timeout)

    def set_world_resume_all(self, return_cmd=False, timeout=5):
        """Wrap the UnrealCV Plus command template ``vset /world/resume_all``."""
        return self._request_plus_template("vset /world/resume_all", (), return_cmd=return_cmd, timeout=timeout)

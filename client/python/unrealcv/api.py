"""UnrealCV API module for interacting with Unreal Engine.

This module provides a high-level interface for communicating with Unreal Engine through UnrealCV.
It includes functionality for:
- Camera control and image capture
- Object manipulation and querying
- Environment control
- Scene configuration

Example:
    >>> from unrealcv import api
    >>> client = api.UnrealCv_API(port=9000, ip='127.0.0.1', resolution=(640, 480))
    >>> # Get an RGB image from camera 0
    >>> image = client.get_image(0, 'lit')
    >>> # Get object names and locations
    >>> objects = client.get_objects()
    >>> for obj in objects:
    >>>     location = client.get_obj_location(obj)
"""

import unrealcv
import cv2
import numpy as np
import math
import time
import os
import re
from io import BytesIO
import PIL.Image
import sys
from unrealcv.util import ResChecker, time_it
import warnings

class UnrealCv_API:
    """
    A class to interact with UnrealCV, a toolkit for using Unreal Engine (UE) in Python.
    """
    def __init__(self, port, ip, resolution, mode='tcp'):
        """
        Initialize the UnrealCV API.

        Args:
            port (int): The port of the UnrealCV Server to connect to.
            ip (str): The IP address of the UnrealCV Server to connect to.
            resolution (tuple): The resolution of the images.
            mode (str): The connection mode, either 'tcp' or 'unix'. Default is 'tcp'. 'unix' is only for local machine in Linux.
        """
        self.ip = ip
        self.resolution = resolution # the resolution is not used.
        self.decoder = MsgDecoder()
        self.checker = ResChecker()
        self.obj_dict = dict()
        self.cam = dict()
        # build a client to connect to the env
        self.client = self.connect(ip, port, mode)
        self.client.message_handler = self.message_handler
        self.init_map()

    def connect(self, ip, port, mode='tcp'):
        """
        Connect to the UnrealCV server.

        Args:
            ip (str): The IP address to connect to.
            port (int): The port to connect to.
            mode (str): The connection mode, either 'tcp' or 'unix'. Default is 'tcp'.

        Returns:
            unrealcv.Client: The connected client.
        """
        client = unrealcv.Client((ip, port))
        client.connect()
        if mode == 'unix':
            if 'linux' in sys.platform and unrealcv.__version__ >= '1.0.0':  # new socket for linux
                unix_socket_path = os.path.join('/tmp/unrealcv_{port}.socket'.format(port=port))  # clean the old socket
                os.remove(unix_socket_path) if os.path.exists(unix_socket_path) else None
                client.disconnect() # disconnect the client for creating a new socket in linux
                time.sleep(2)
                if unix_socket_path is not None and os.path.exists(unix_socket_path):
                    client = unrealcv.Client(unix_socket_path, 'tcp')
                else:
                    client = unrealcv.Client((ip, port)) # reconnect to the tcp socket
                client.connect()
            else:
                warnings.warn('unix socket mode is not supported in this platform, switch to tcp mode.')
        return client

    def init_map(self):
        """
        Initialize the map by getting camera configurations and building a color dictionary for objects.
        """
        self.cam = self.get_camera_config()
        self.obj_dict = self.build_color_dict(self.get_objects())

    def camera_info(self):
        """
        Get the camera information.

        Returns:
            dict: The camera information.
        """
        return self.cam

    def config_ue(self, resolution=(320, 240), low_quality=False, disable_all_screen_messages=True):
        """
        Configure Unreal Engine settings.

        Args:
            resolution (tuple): The resolution of the display window.
            low_quality (bool): Whether to set the rendering quality to low. Default is False.
            disable_all_screen_messages (bool): Whether to disable all screen messages. Default is True.
        """
        self.check_connection()
        [w, h] = resolution
        self.client.request(f'vrun setres {w}x{h}w', -1)  # set resolution of the display window
        if disable_all_screen_messages:
            self.client.request('DisableAllScreenMessages', -1)  # disable all screen messages
        if low_quality:
            self.client.request('vrun sg.ShadowQuality 0', -1)  # set shadow quality to low
            self.client.request('vrun sg.TextureQuality 0', -1)  # set texture quality to low
            self.client.request('vrun sg.EffectsQuality 0', -1)  # set effects quality to low
        time.sleep(0.1)

    def message_handler(self, message):
        """
        Handle messages from the UnrealCV server.

        Args:
            message (str): The message from the server.
        """
        msg = message
        print(msg)

    def check_connection(self):
        """
        Check the connection to the UnrealCV server and reconnect if necessary.
        """
        while self.client.isconnected() is False:
            warnings.warn('UnrealCV server is not running. Please try again')
            time.sleep(1)
            self.client.connect()

    def get_camera_config(self):
        """
        Get the camera configuration(the location, rotation, and fov of each camera).

        Returns:
            dict: The camera configuration.
        """
        num_cameras = self.get_camera_num()
        cam = dict()
        for i in range(num_cameras):
            cam[i] = dict(
                 location=self.get_cam_location(i, syns=False),
                 rotation=self.get_cam_rotation(i, syns=False),
                 fov=self.get_cam_fov(i)
            )
        return cam

    def get_objects(self):
        """
        Get all object names in the map.

        Returns:
            list: The list of object names.
        """
        objects = self.client.request('vget /objects').split()
        return objects

    # batch_functions for multiple commands
    def batch_cmd(self, cmds, decoders, **kwargs):
        """
        Execute a batch of commands.

        Args:
            cmds (list): The list of commands.
            decoders (list): The list of decoder functions.
            **kwargs: Additional arguments for the decoder functions.

        Returns:
            list: The list of results.
        Examples:
            >>> cmds = ['vget /camera/0/rotation', 'vget /camera/0/location']
            >>> decoders = [self.decoder.string2floats, self.decoder.string2floats]
            >>> results = self.batch_cmd(cmds, decoders)
            >>> print(results)  # [[0, 0, 90], [100.0, 200.0, 300.0]]
        """

        res_list = self.client.request(cmds)
        if decoders is None: # vset commands do not decode return
            return res_list
        for i, res in enumerate(res_list):
            res_list[i] = decoders[i](res, **kwargs)
        return res_list

    def save_image(self, cam_id, viewmode, path, return_cmd=False):
        """
        Save an image from a camera.

        Args:
            cam_id (int): The camera ID.
            viewmode (str): The view mode (e.g., 'lit', 'depth').
            path (str): The path to save the image.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            str: The command if return_cmd is True, otherwise the image directories.
        """
        cmd = f'vget /camera/{cam_id}/{viewmode} {path}'
        if return_cmd:
            return cmd
        # check file extension
        try:
            if viewmode == 'depth':
                expect_extension = 'npy'
            else:
                expect_extension = ['bmp', 'png']
            if not self.checker.is_expected_file_extension(path, expect_extension):
                raise ValueError(f'Invalid file extension for {viewmode} image, it should be {expect_extension}',)
        except ValueError as e:
            if viewmode == 'depth':
                path += '.npy'
            else:
                path += '.png'

        self.client.request(cmd)
        img_dirs = self.client.request(cmd)

        return img_dirs

    def get_image(self, cam_id, viewmode, mode='bmp', return_cmd=False, show=False, inverse=False):
        """
        Get an image from a camera.

        Args:
            cam_id (int): The camera ID.
            viewmode (str): The view mode (e.g., 'lit', 'normal', 'object_mask', 'depth').
            mode (str): The image format (e.g., 'bmp', 'png', 'npy'). Default is 'bmp'.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
            show (bool): Whether to display the image. Default is False.
            inverse (bool): Whether to inverse the depth. Default is False.
        Returns:
            np.ndarray: The image.
        """
        if viewmode == 'depth':
            return self.get_depth(cam_id, return_cmd=return_cmd, show=show)
        cmd = f'vget /camera/{cam_id}/{viewmode} {mode}'
        if return_cmd:
            return cmd
        image = self.decoder.decode_img(self.client.request(cmd), mode, inverse)
        if show:
            cv2.imshow('image_'+viewmode, image)
            cv2.waitKey(1)
        return image

    def get_depth(self, cam_id, inverse=False, return_cmd=False, show=False):  # get depth from unrealcv in npy format
        """
        Get the depth image from a camera.

        Args:
            cam_id (int): The camera ID.
            inverse (bool): Whether to inverse the depth. Default is False.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
            show (bool): Whether to display the image. Default is False.

        Returns:
            np.ndarray: The depth image.
        """
        cmd = f'vget /camera/{cam_id}/depth npy'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        depth = self.decoder.decode_depth(res, inverse)
        if show:
            cv2.imshow('image', depth/depth.max())  # normalize the depth image
            cv2.waitKey(10)
        return depth
    
    def get_optical_flow(self, cam_id, return_cmd=False, show=False):
        cmd = f'vget /camera/{cam_id}/optical_flow bmp'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        optical_flow = self.decoder.decode_bmp(res)
        if show:
            cv2.imshow('image', optical_flow)
            cv2.waitKey(1)
        return optical_flow

    def get_image_multicam(self, cam_ids, viewmode='lit', mode='bmp', inverse=True):
        """
        Get images from multiple cameras with the same view mode.

        Args:
            cam_ids (list): The list of camera IDs.
            viewmode (str): The view mode (e.g., 'lit', 'depth', 'normal', 'object_mask'). Default is 'lit'.
            mode (str): The image format (e.g., 'bmp', 'npy', 'png'). Default is 'bmp'.
            inverse (bool): Whether to inverse the depth. Default is True.

        Returns:
            list: The list of images.
        """
        cmds = [self.get_image(cam_id, viewmode, mode, return_cmd=True) for cam_id in cam_ids]
        decoders = [self.decoder.decode_img for i in cam_ids]
        img_list = self.batch_cmd(cmds, decoders, mode=mode, inverse=inverse)
        return img_list

    def get_image_multimodal(self, cam_id, viewmodes=None, modes=None): # get rgb and depth image
        """
        Get multimodal images from a camera, such as RGB-D images (default).

        Args:
            cam_id (int): The camera ID.
            viewmodes (list): The list of view modes. Default is ['lit', 'depth'].
            modes (list): The list of image formats. Default is ['bmp', 'npy'].

        Returns:
            np.ndarray: The concatenated image.
        """
        if viewmodes is None:
            viewmodes = ['lit', 'depth']
        if modes is None:
            modes = ['bmp', 'npy']
        cmds = [self.get_image(cam_id, viewmode, mode, return_cmd=True) for viewmode, mode in zip(viewmodes, modes)]
        decoders = [self.decoder.decode_map[mode] for mode in modes]
        res = self.batch_cmd(cmds, decoders)
        concat_img = np.concatenate(res, axis=2)
        return concat_img

    def get_img_batch(self, cam_info):
        """
        Get multiple images from multiple cameras or viewmodes, the most flexible function to get images.

        Args:
            cam_info (dict): The camera information, such as {cam_id: {viewmode: {'mode': 'bmp', 'inverse': True, 'img': None}}}

        Returns:
            dict: The updated camera information with images.
        """
        cmd_list = []
        # prepare command list
        for cam_id in cam_info.keys():
            for viewmode in cam_info[cam_id].keys():
                mode = cam_info[cam_id][viewmode]['mode']
                cmd_list.append(self.get_image(cam_id, viewmode, mode, return_cmd=True))

        res_list = self.client.request(cmd_list)
        # decode images and store in cam_info
        for cam_id in cam_info.keys():
            for viewmode in cam_info[cam_id].keys():
                mode = cam_info[cam_id][viewmode]['mode']
                inverse = cam_info[cam_id][viewmode]['inverse']
                cam_info[cam_id][viewmode]['img'] = self.decoder.decode_img(res_list.pop(0), mode, inverse)
        return cam_info


    def set_cam_pose(self, cam_id, pose):
        """
        Set the camera pose.

        Args:
            cam_id (int): The camera ID.
            pose (list): The camera pose [x, y, z, pitch, yaw, roll].
        """
        [x, y, z, roll, yaw, pitch] = pose
        self.set_cam_rotation(cam_id, [roll, yaw, pitch])
        self.set_cam_location(cam_id, [x, y, z])
        # cmd = f'vset /camera/{cam_id}/pose {x} {y} {z} {pitch} {yaw} {roll}'
        # self.client.request(cmd, -1)
        self.cam[cam_id]['location'] = [x, y, z]
        self.cam[cam_id]['rotation'] = [roll, yaw, pitch]

    def get_cam_pose(self, cam_id, mode='hard'):  # get camera pose, pose = [x, y, z, roll, yaw, pitch]
        """
        Get the camera pose. The mode can be 'hard' or 'soft'. The 'hard' mode will get the pose from UnrealCV, while the 'soft' mode will get the pose from self.cam.

        Args:
            cam_id (int): The camera ID.
            mode (str): The mode to get the pose, either 'hard' or 'soft'. Default is 'hard'.

        Returns:
            list: The camera pose [x, y, z, roll, yaw, pitch].
        """
        if mode == 'soft':
            pose = self.cam[cam_id]['location']
            pose.extend(self.cam[cam_id]['rotation'])
            return pose
        if mode == 'hard':
            cmds = [self.get_cam_location(cam_id, return_cmd=True), self.get_cam_rotation(cam_id, return_cmd=True)]
            decoders = [self.decoder.decode_map[self.decoder.cmd2key(cmd)] for cmd in cmds]
            res = self.batch_cmd(cmds, decoders)
            self.cam[cam_id]['location'] = res[0]
            self.cam[cam_id]['rotation'] = res[1]
            return res[0] + res[1]

    def set_cam_fov(self, cam_id, fov):
        """
        Set the camera field of view (fov).

        Args:
            cam_id (int): The camera ID.
            fov (float): The field of view.
        
        Example:
            >>> api.set_cam_fov(0, 90) # set the fov of the camera 0 to 90 degrees.
        """
        if fov == self.cam[cam_id]['fov']:
            return fov

        cmd = f'vset /camera/{cam_id}/fov {fov}'
        self.client.request(cmd, -1)
        self.cam[cam_id]['fov'] = fov
        return fov

    def get_cam_fov(self, cam_id):
        """
        Get the camera field of view (fov).

        Args:
            cam_id (int): The camera ID.

        Returns:
            float: The field of view.
        """
        cmd = f'vget /camera/{cam_id}/fov'
        fov = self.client.request(cmd)
        return fov

    def set_cam_location(self, cam_id, loc):
        """
        Set the camera location.

        Args:
            cam_id (int): The camera ID.
            loc (list): The camera location [x, y, z].
        """
        [x, y, z] = loc
        cmd = f'vset /camera/{cam_id}/location {x} {y} {z}'
        self.client.request(cmd, -1)
        self.cam[cam_id]['location'] = loc

    def get_cam_location(self, cam_id, newest=True, return_cmd=False, syns=True):
        """
        Get the camera location.

        Args:
            cam_id (int): The camera ID.
            newest (bool): Whether to get the newest location from UnrealCV. Default is True.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
            syns (bool): Whether to synchronize the location with the internal state. Default is True.

        Returns:
            list: The camera location [x, y, z].
        """
        if newest:
            cmd = f'vget /camera/{cam_id}/location'
            if return_cmd:
                return cmd
            res = None
            while res is None:
                res = self.client.request(cmd)
            res = self.decoder.string2floats(res)
            if syns:
                self.cam[cam_id]['location'] = res
        else:
            return self.cam[cam_id]['location']
        return res

    def set_cam_rotation(self, cam_id, rot, rpy=False):
        """
        Set the camera rotation.

        Args:
            cam_id (int): The camera ID.
            rot (list): The camera rotation [roll, yaw, pitch].
            rpy (bool): Whether the rotation is in roll-pitch-yaw format. Default is False.
        """
        if rpy:
            [roll, yaw, pitch] = rot
        else:
            [pitch, yaw, roll] = rot
        cmd = f'vset /camera/{cam_id}/rotation {pitch} {yaw} {roll}'
        self.client.request(cmd, -1)
        self.cam[cam_id]['rotation'] = [pitch, yaw, roll]

    def get_cam_rotation(self, cam_id, newest=True, return_cmd=False, syns=True):
        """
        Get the camera rotation.

        Args:
            cam_id (int): The camera ID.
            newest (bool): Whether to get the newest rotation from UnrealCV. Default is True.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
            syns (bool): Whether to synchronize the rotation with the internal state. Default is True.

        Returns:
            list: The camera rotation [pitch, yaw, roll].
        """
        if newest:
            cmd = f'vget /camera/{cam_id}/rotation'
            if return_cmd:
                return cmd
            res = None
            while res is None:
                res = self.client.request(cmd)
            res = [float(i) for i in res.split()]
            if syns:
                self.cam[cam_id]['rotation'] = res
            return res
        else:
            return self.cam[cam_id]['rotation']

    def move_cam(self, cam_id, loc):
        """
        Move the camera to a location with physics simulation.

        Args:
            cam_id (int): The camera ID.
            loc (list): The target location [x, y, z].
        """
        [x, y, z] = loc
        cmd = f'vset /camera/{cam_id}/moveto {x} {y} {z}'
        self.client.request(cmd)

    def move_cam_forward(self, cam_id, yaw, distance, height=0, pitch=0):
        """
        Move the camera forward as a mobile robot at specific height.

        Args:
            cam_id (int): The camera ID.
            yaw (float): The delta angle between the camera and the x-axis.
            distance (float): The absolute distance from the last location to the next location.
            height (float): The height of the camera. Default is 0.
            pitch (float): The pitch angle. Default is 0.

        Returns:
            bool: Whether the movement was successful.
        """
        yaw_exp = (self.cam[cam_id]['rotation'][1] + yaw) % 360
        pitch_exp = (self.cam[cam_id]['rotation'][0] + pitch) % 360
        assert abs(height) < distance, 'height should be smaller than distance'
        if height != 0:
            distance_plane = np.sqrt(distance**2 - height**2)
        else:
            distance_plane = distance
        delt_x = distance_plane * math.cos(yaw_exp / 180.0 * math.pi)
        delt_y = distance_plane * math.sin(yaw_exp / 180.0 * math.pi)

        location_now = self.get_cam_location(cam_id)
        location_exp = [location_now[0] + delt_x, location_now[1]+delt_y, location_now[2]+height]

        self.move_cam(cam_id, location_exp)
        if yaw != 0 or pitch != 0:
            self.set_cam_rotation(cam_id, [0, yaw_exp, pitch_exp])

        location_now = self.get_cam_location(cam_id)
        error = self.get_distance(location_now, location_exp, 3)

        if error < 10:
            return False
        else:
            return True

    def get_distance(self, pos_now, pos_exp, n=2):  # get distance between two points, n is the dimension
        """
        Get the distance between two points.

        Args:
            pos_now (list): The current position.
            pos_exp (list): The expected position.
            n (int): The dimension. Default is 2.

        Returns:
            float: The distance between the two points.
        """
        error = np.array(pos_now[:n]) - np.array(pos_exp[:n])
        distance = np.linalg.norm(error)
        return distance

    def set_keyboard(self, key, duration=0.01):
        """
        Simulate a keyboard action.

        Args:
            key (str): The key to press, such as 'Up', 'Down', 'Left', 'Right'
            duration (float): The duration to press the key. Default is 0.01.

        Example:
            >>> api.set_keyboard('Up', 0.1) # press the up key for 0.1 seconds
            >>> api.set_keyboard('Left', 0.1) # press the left key for 0.1 seconds
        """
        cmd = 'vset /action/keyboard {key} {duration}'
        self.client.request(cmd.format(key=key, duration=duration), -1)

    def get_obj_color(self, obj, return_cmd=False):
        """
        Get the color of an object in the object mask.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The color of the object [r, g, b].
        """
        cmd = f'vget /object/{obj}/color'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        return self.decoder.string2color(res)[:-1]

    def set_obj_color(self, obj, color, return_cmd=False):  # set object color in object mask, color = [r,g,b]
        """
        Set the color of an object to render in the object mask.

        Args:
            obj (str): The object name.
            color (list): The color to set [r, g, b].
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        
        Example:
            >>> api.set_obj_color('tree', [255, 0, 0]) # set the color of the tree to red
        """
        [r, g, b] = color
        cmd = f'vset /object/{obj}/color {r} {g} {b}'
        self.obj_dict[obj] = color
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)  # -1 means async mode

    def set_obj_location(self, obj, loc):
        """
        Set the location of an object in the scene with async mode.

        Args:
            obj (str): The object name.
            loc (list): The location to set [x, y, z].

        Example:
            >>> api.set_obj_location('tree', [100, 200, 300]) # set the location of the tree to [100, 200, 300]
        """
        [x, y, z] = loc
        cmd = f'vset /object/{obj}/location {x} {y} {z}'
        self.client.request(cmd, -1)  # -1 means async mode


    def set_obj_rotation(self, obj, rot):
        """
        Set the rotation of an object in the scene with async mode.

        Args:
            obj (str): The object name.
            rot (list): The rotation to set [roll, yaw, pitch].
        """
        [roll, yaw, pitch] = rot
        cmd = f'vset /object/{obj}/rotation {pitch} {yaw} {roll}'
        self.client.request(cmd, -1)

    def get_mask(self, object_mask, obj, threshold=3):
        """
        Get the mask of a specific object in the object mask image.

        Args:
            object_mask (np.ndarray): The object mask image.
            obj (str): The object name.
            threshold (int): The color threshold. Default is 3.

        Returns:
            np.ndarray: The mask of the object.
        """
        [r, g, b] = self.obj_dict[obj]
        lower_range = np.array([b-threshold, g-threshold, r-threshold])
        upper_range = np.array([b+threshold, g+threshold, r+threshold])
        mask = cv2.inRange(object_mask, lower_range, upper_range)
        return mask


    def get_bbox(self, object_mask, obj, normalize=True):
        """
        Get the bounding box of an object.

        Args:
            object_mask (np.ndarray): The object mask image.
            obj (str): The object name.
            normalize (bool): Whether to normalize the bounding box coordinates. Default is True.

        Returns:
            tuple: The mask and bounding box of the object.

        Example:
            >>> object_mask = api.get_image(0, 'mask')
            >>> mask, box = api.get_bbox(object_mask, 'tree')
        """
        width = object_mask.shape[1]
        height = object_mask.shape[0]
        mask = self.get_mask(object_mask, obj)
        pixelpointsCV2 = cv2.findNonZero(mask)


        if isinstance(pixelpointsCV2, np.ndarray):  # exist target in image
            x_min = pixelpointsCV2[:, :, 0].min()
            x_max = pixelpointsCV2[:, :, 0].max()
            y_min = pixelpointsCV2[:, :, 1].min()
            y_max = pixelpointsCV2[:, :, 1].max()
            if normalize:
                box = ((x_min/float(width), y_min/float(height)),  # left top
                       (x_max/float(width), y_max/float(height)))  # right down
            else:
                box = [x_min, y_min, x_max-x_min, y_max-y_min]
        else:
            if normalize:
                box = ((0, 0), (0, 0))
            else:
                box = [0, 0, 0, 0]

        return mask, box

    def get_obj_bboxes(self, object_mask, objects, return_dict=False):
        """
        Get the bounding boxes of multiple objects.

        Args:
            object_mask (np.ndarray): The object mask image.
            objects (list): The list of object names.
            return_dict (bool): Whether to return the bounding boxes as a dictionary. Default is False.

        Returns:
            list or dict: The list or dictionary of bounding boxes.
        """
        boxes = []
        for obj in objects:
            mask, box = self.get_bbox(object_mask, obj)
            boxes.append(box)
        if return_dict:
            return dict(zip(objects, boxes))
        else:
            return boxes

    def build_color_dict(self, objects, batch=True):  # build a color dictionary for objects
        """
        Build a color dictionary for objects.

        Args:
            objects (list): The list of object names.
            batch (bool): Whether to use batch commands. Default is True.

        Returns:
            dict: The color dictionary.

        Example:
            >>> objects = ['tree', 'house']
            >>> color_dict = api.build_color_dict(objects)
            >>> color_dict['tree'] = [255, 0, 0]
            >>> color_dict['house'] = [0, 255, 0]
        """
        color_dict = dict()
        if batch:
            cmds = [self.get_obj_color(obj, return_cmd=True) for obj in objects]
            decoders = [self.decoder.string2color for _ in objects]
            res = self.batch_cmd(cmds, decoders)
        else:
            res = [self.get_obj_color(obj) for obj in objects]
        for obj, color in zip(objects, res):
            color_dict[obj] = color
        self.obj_dict = color_dict
        return color_dict

    def get_obj_location(self, obj, return_cmd=False):  # get object location
        """
        Get the location of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The location of the object [x, y, z].
        """
        cmd = f'vget /object/{obj}/location'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return self.decoder.string2floats(res)

    def get_obj_rotation(self, obj, return_cmd=False):  # get object rotation
        """
        Get the rotation of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The rotation of the object [roll, yaw, pitch].
        """
        cmd = f'vget /object/{obj}/rotation'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return self.decoder.string2floats(res)

    def get_obj_pose(self, obj):
        """
        Get the pose of an object.

        Args:
            obj (str): The object name.

        Returns:
            list: The pose of the object [x, y, z, roll, yaw, pitch].
        """
        cmds = [self.get_obj_location(obj, return_cmd=True), self.get_obj_rotation(obj, return_cmd=True)]
        decoders = [self.decoder.decode_map[self.decoder.cmd2key(cmd)] for cmd in cmds]
        res = self.batch_cmd(cmds, decoders)
        return res[0] + res[1]

    def build_pose_dic(self, objects):  # build a pose dictionary for objects
        """
        Build a pose dictionary for objects.

        Args:
            objects (list): The list of object names.

        Returns:
            dict: The pose dictionary.
        """
        pose_dic = dict()
        for obj in objects:
            pose = self.get_obj_location(obj)
            pose.extend(self.get_obj_rotation(obj))
            pose_dic[obj] = pose
        return pose_dic

    def get_obj_bounds(self, obj, return_cmd=False): # get object location
        """
        Get the bounds of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The bounds of the object [min_x, min_y, min_z, max_x, max_y, max_z].
        """
        cmd = f'vget /object/{obj}/bounds'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return self.decoder.string2floats(res)  # min x,y,z  max x,y,z

    def get_obj_size(self, obj, box=True):
        """
        Get the size of an object.

        Args:
            obj (str): The object name.
            box (bool): Whether to return the bounding box size. Default is True.

        Returns:
            list or float: The [size_x, size_y, size_z] or volume (float) of the 3D bounding box.
        """
        self.set_obj_rotation(obj, [0, 0, 0])  # init
        bounds = self.get_obj_bounds(obj)
        x = bounds[3] - bounds[0]
        y = bounds[4] - bounds[1]
        z = bounds[5] - bounds[2]
        if box:
            return [x, y, z]
        else:
            return x*y*z

    def get_obj_scale(self, obj, return_cmd=False):
        """
        Get the scale of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The scale of the object [scale_x, scale_y, scale_z].
        """
        cmd = f'vget /object/{obj}/scale'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return self.decoder.string2floats(res)  # [scale_x, scale_y, scale_z]

    def set_obj_scale(self, obj, scale=None, return_cmd=False):
        """
        Set the scale of an object.

        Args:
            obj (str): The object name.
            scale (list): The scale to set [scale_x, scale_y, scale_z]. Default is [1, 1, 1].
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        """
        if scale is None:
            scale = [1, 1, 1]
        [x, y, z] = scale
        cmd = f'vset /object/{obj}/scale {x} {y} {z}'
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)

    def set_hide_obj(self, obj, return_cmd=False):
        """
        Hide an object, making it invisible but still present in the physics engine.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        """
        cmd = f'vset /object/{obj}/hide'
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)

    def set_show_obj(self, obj, return_cmd=False):
        """
        Show an object, making it visible.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        """
        cmd = f'vset /object/{obj}/show'
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)

    def set_hide_objects(self, objects):
        """
        Hide multiple objects, making them invisible but still present in the physics engine.

        Args:
            objects (list): The list of object names.
        """
        cmds = [self.set_hide_obj(obj, return_cmd=True) for obj in objects]
        self.client.request(cmds, -1)

    def set_show_objects(self, objects):
        """
        Show multiple objects, making them visible.

        Args:
            objects (list): The list of object names.
        """
        cmds = [self.set_show_obj(obj, return_cmd=True) for obj in objects]
        self.client.request(cmds, -1)

    def destroy_obj(self, obj):
        """
        Destroy an object, removing it from the scene.

        Args:
            obj (str): The object name.
        """
        self.client.request(f'vset /object/{obj}/destroy', -1)
        self.obj_dict.pop(obj)
        # TODO: remove the cameras mounted at the object

    def get_camera_num(self):
        """
        Get the number of cameras.

        Returns:
            int: The number of cameras.
        """
        res = self.client.request('vget /cameras')
        return len(res.split())

    def get_camera_list(self):
        """
        Get the list of cameras.

        Returns:
            list: The list of camera names.
        """
        res = self.client.request('vget /cameras')
        return res.split()

    def set_new_camera(self):
        """
        Spawn a new camera.

        Returns:
            str: The object name of the new camera.
        """
        res = self.client.request('vset /cameras/spawn')
        cam_id = len(self.cam)
        self.register_camera(cam_id)
        return res

    def register_camera(self, cam_id, obj_name=None):
        """
        Register a camera in self.cam (dict).

        Args:
            cam_id (int): The camera ID.
            obj_name (str): The object name. Default is None.
        """
        self.cam[cam_id] = dict(
            obj_name=obj_name,
            location=self.get_cam_location(cam_id, syns=False),
            rotation=self.get_cam_rotation(cam_id, syns=False),
            fov=self.get_cam_fov(cam_id),
        )

    def set_new_obj(self, class_name, obj_name):
        """
        Spawn a new object.

        Args:
            class_name (str): The class name of the object.
            obj_name (str): The object name.

        Returns:
            str: The object name of the new object.
        
        Example:
            >>> api.set_new_obj('Cube_C', 'cube_1')
        """
        cmd = f'vset /objects/spawn {class_name} {obj_name}'
        res = self.client.request(cmd)
        if self.checker.is_error(res):
            hint = (
                f"{res}. Hint: set_new_obj() prefers UClass names. "
                f"If you already have an asset path like /Game/... or /Engine/... , use spawn_object_from_path()."
            )
            if isinstance(class_name, str) and class_name.startswith('/'):
                try:
                    return self.spawn_object_from_path(
                        class_name, obj_name=obj_name, annotate=True
                    )
                except Exception as exc:
                    warnings.warn(
                        f"{hint} Auto-fallback to spawn_object_from_path({class_name}, {obj_name}) "
                        f"also failed: {exc}"
                    )
                    return None
            warnings.warn(hint)
            return None
        else:  # add object to the object list, check if new cameras are added
            # assign a random color to the object
            color = np.random.randint(0, 255, 3).tolist()
            used_colors = [list(c) for c in self.obj_dict.values()]
            while color in used_colors:
                color = np.random.randint(0, 255, 3).tolist()
            self.obj_dict[obj_name] = color
            self.set_obj_color(obj_name, color)
            # check if new cameras are added
            while len(self.cam) < self.get_camera_num():
                self.register_camera(len(self.cam), obj_name)
            return obj_name

    def spawn_object_from_path(self, asset_path, obj_name=None, annotate=True, return_cmd=False):
        """
        Spawn an object directly from an asset path.

        Args:
            asset_path (str): Full asset path, for example ``/Game/Props/Chair.Chair``.
            obj_name (str | None): Optional spawned actor name.
            annotate (bool): Whether to use the auto-annotation variant. Default is True.
            return_cmd (bool): Whether to return the command string instead of executing it.

        Returns:
            str: Spawned object name.
        """
        cmd_name = 'spawn_from_path' if annotate else 'spawn_from_path_wo_annotation'
        cmd = f'vset /objects/{cmd_name} {asset_path}'
        if obj_name is not None:
            cmd += f' {obj_name}'
        if return_cmd:
            return cmd

        res = self.client.request(cmd)
        if self.checker.is_error(res):
            warnings.warn(res)
            return None

        spawned_name = res.strip()
        if annotate:
            try:
                self.obj_dict[spawned_name] = self.get_obj_color(spawned_name)
            except Exception:
                pass
        return spawned_name

    def get_vertex_locations(self, obj, return_cmd=False):
        """
        Get the vertex locations of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            list: The vertex locations.
        """
        cmd = f'vget /object/{obj}/vertex_location'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return self.decoder.decode_vertex(res)

    def get_obj_uclass(self, obj, return_cmd=False):
        """
        Get the Unreal class name of an object.

        Args:
            obj (str): The object name.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.

        Returns:
            str: The Unreal class name of the object.
        """
        cmd = f'vget /object/{obj}/uclass_name'
        if return_cmd:
            return cmd
        res = None
        while res is None:
            res = self.client.request(cmd)
        return res

    def set_map(self, map_name, return_cmd=False):  # change to a new level map
        """
        Change to a new level map.

        Args:
            map_name (str): The name of the map.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        """
        cmd = f'vset /action/game/level {map_name}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if self.checker.not_error(res):
            self.init_map()

    def set_pause(self, return_cmd=False):
        """
        Pause the game simulation.

        Args:
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        
        Example:
            >>> api.set_pause() # every object will not move
        """
        cmd = f'vset /action/game/pause'
        if return_cmd:
            return cmd

        self.client.request(cmd)

    def set_resume(self, return_cmd=False):
        """
        Resume the game simulation.

        Args:
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        
        Example:
            >>> api.set_resume() # every object will move again.
        """
        cmd = f'vset /action/game/resume'
        if return_cmd:

            return cmd
        self.client.request(cmd)

    def get_is_paused(self):
        """
        Check if the game is paused.

        Returns:
            bool: True if the game is paused, False otherwise.
        """
        res = self.client.request('vget /action/game/is_paused')
        return res == 'true'

    def set_global_time_dilation(self, time_dilation, return_cmd=False):
        """
        Set the global time dilation, which affects the simulation speed of the game.

        Args:
            time_dilation (float): The time dilation factor.
            return_cmd (bool): Whether to return the command instead of executing it. Default is False.
        """
        cmd = f'vrun slomo {time_dilation}'
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)

    def set_max_FPS(self, max_fps, return_cmd=False):
        """
        Set the maximum frames per second (FPS) for the Unreal Engine.

        Args:
            max_fps (int): The maximum FPS to set.
            return_cmd (bool): Whether to return the command string instead of executing it. Default is False.

        Returns:
            str: The command string if return_cmd is True, otherwise None.
        """
        cmd = f'vrun t.maxFPS {max_fps}'
        if return_cmd:
            return cmd
        self.client.request(cmd, -1)

#########################################################################################################################
# Recording APIs
#########################################################################################################################

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
        res = self.client.request(cmd)
        if res.isdigit():
            return int(res)
        raise ValueError(f"Error: {res}")

    def set_recording_time_dilation(self, dilation, return_cmd=False):
        cmd = f'vset /captureactor/time_dilation {dilation}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd, -1)
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
        res = self.client.request(cmd)
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
        return self.decoder.string2bool(self.client.request(cmd))

#########################################################################################################################
# Latest UnrealCV+ APIs
#########################################################################################################################

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
        return self.client.request(cmd).split()

    def get_camera_list_cid(self, return_cmd=False):
        """
        Get stable UnrealCV+ camera identifiers (CID format).
        """
        cmd = 'vget /cameras_CID'
        if return_cmd:
            return cmd
        return self.client.request(cmd).split()

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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        return self._parse_bool_response(self.client.request(cmd))

    def get_pak_files(self, pak_file_path, return_cmd=False):
        """
        List raw file entries recorded in a pak file index.
        """
        cmd = f'vget /pak/files {pak_file_path}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
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
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def capture_panoramic(self, cam_id, path, width=None, height=None, return_cmd=False):
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
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def stop_recording(self, cam_id, return_cmd=False):
        """
        Stop an active recording for a camera.
        """
        cmd = f'vset /captureactor/{cam_id}/stop_record'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_use_movie_quality_rendering(self, return_cmd=False):
        cmd = 'vget /captureactor/use_movie_quality_rendering'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self.client.request(cmd))

    def set_use_movie_quality_rendering(self, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/use_movie_quality_rendering {enabled}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_record_via_viewport(self, return_cmd=False):
        cmd = 'vget /captureactor/record_via_viewport'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self.client.request(cmd))

    def set_record_via_viewport(self, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/record_via_viewport {enabled}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_warmup_frames(self, return_cmd=False):
        cmd = 'vget /captureactor/warmup_frames'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if not res.isdigit():
            raise ValueError(f"Error: {res}")
        return int(res)

    def set_warmup_frames(self, warmup_frames, return_cmd=False):
        cmd = f'vset /captureactor/warmup_frames {warmup_frames}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_paused_tick_interval(self, return_cmd=False):
        cmd = 'vget /captureactor/paused_tick_interval'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return float(res)

    def set_paused_tick_interval(self, tick_interval, return_cmd=False):
        cmd = f'vset /captureactor/paused_tick_interval {tick_interval}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_record_add_timestamp(self, cam_id, return_cmd=False):
        cmd = f'vget /captureactor/{cam_id}/add_timestamp'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self.client.request(cmd))

    def set_record_add_timestamp(self, cam_id, enabled, return_cmd=False):
        enabled = self._to_uint_flag(enabled)
        cmd = f'vset /captureactor/{cam_id}/add_timestamp {enabled}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res

    def get_recording_paused(self, cam_id, return_cmd=False):
        cmd = f'vget /captureactor/{cam_id}/paused'
        if return_cmd:
            return cmd
        return self._parse_bool_response(self.client.request(cmd))

    def set_recording_paused(self, cam_id, paused, return_cmd=False):
        paused = self._to_uint_flag(paused)
        cmd = f'vset /captureactor/{cam_id}/paused {paused}'
        if return_cmd:
            return cmd
        res = self.client.request(cmd)
        if res.startswith("error"):
            raise ValueError(res)
        return res


class MsgDecoder:
    """
    Message decoder for UnrealCV server responses.

    This class provides methods to decode various types of responses from the UnrealCV server,
    including images, vectors, colors, and other data formats.

    Attributes:
        decode_map (dict): Mapping of data types to their decoder functions:
            - vertex_location: Decodes vertex coordinates
            - color: Decodes RGB color values
            - rotation: Decodes rotation angles
            - location: Decodes position coordinates
            - bounds: Decodes bounding box coordinates
            - scale: Decodes scale factors
            - png: Decodes PNG images
            - bmp: Decodes BMP images
            - npy: Decodes NumPy array data

    Example:
        >>> decoder = MsgDecoder()
        >>> # Decode a color string
        >>> color = decoder.string2color("(R=255,G=128,B=64)")
        >>> print(color)  # [255, 128, 64]
        >>>
        >>> # Decode position coordinates
        >>> pos = decoder.string2floats("100.0 200.0 300.0")
        >>> print(pos)  # [100.0, 200.0, 300.0]
    """
    def __init__(self):
        """
        Initialize the decoder with mapping of data types to decoder functions.
        """
        self.decode_map = {
            'vertex_location': self.decode_vertex,
            'color': self.string2color,
            'rotation': self.string2floats,
            'location': self.string2floats,
            'bounds': self.string2floats,
            'scale': self.string2floats,
            'png': self.decode_png,
            'bmp': self.decode_bmp,
            'npy': self.decode_npy
        }

    def cmd2key(self, cmd):  # extract the last word of the command as key
        """
        Extract the last word of the command as key.

        Args:
            cmd (str): The command string.

        Returns:
            str: The key extracted from the command.
        """
        return re.split(r'[/\s]+', cmd)[-1]

    def decode(self, cmd, res):
        """Universal decode function that selects appropriate decoder based on command.

        Args:
            cmd (str): Command string.
            res (str): Response data.

        Returns:
            Any: Decoded data in appropriate format.

        Example:
            >>> res = decoder.decode("vget /object/cube/color", "(R=255,G=128,B=64)")
            >>> print(res)  # [255, 128, 64]
        """
        key = self.cmd2key(cmd)
        decode_func = self.decode_map.get(key)
        return decode_func(res)

    def string2bool(self, res):
        bool_map = {
            "True": True,
            "False": False,
            "true": True,
            "false": False,
        }
        value = bool_map.get(res)
        if value is None:
            raise ValueError(f"Invalid boolean value: {res}")
        return value

    def string2list(self, res):
        """
        Convert a string to a list.

        Args:
            res (str): The response string.

        Returns:
            list: The list of strings.
        """
        return res.split()

    def string2floats(self, res):
        """Convert space-separated string of numbers to float list.

        Args:
            res (str): Space-separated numbers string.

        Returns:
            list[float]: List of floating point numbers.

        Example:
            >>> floats = decoder.string2floats("1.0 2.5 3.7")
            >>> print(floats)  # [1.0, 2.5, 3.7]
        """
        return [float(i) for i in res.split()]

    def string2color(self, res):
        """Decode color string to RGB values.

        Args:
            res (str): Color string in format "(R=255,G=128,B=64)".

        Returns:
            list[int]: RGB color values as [r, g, b].

        Example:
            >>> color = decoder.string2color("(R=255,G=128,B=64)")
            >>> print(color)  # [255, 128, 64]
        """
        object_rgba = re.findall(r"\d+\.?\d*", res)
        color = [int(i) for i in object_rgba]  # [r,g,b,a]
        return color[:-1]  # [r,g,b]

    def string2vector(self, res):
        """
        Decode a vector string.

        Args:
            res (str): The response string.

        Returns:
            list: The vector as a list of floats.
        """
        res = re.findall(r"[+-]?\d+\.?\d*", res)
        vector = [float(i) for i in res]
        return vector

    def bpstring2floats(self, res):
        """
        Decode a string of numbers into a list of floats for blueprint.

        Args:
            res (str): The response string.

        Returns:
            list or float: The list of floats or a single float.
        """
        valuse = re.findall(r'"([\d]+\.?\d*)"', res)
        if len(valuse) == 1:
            return float(valuse[0])
        else:
            return [float(i) for i in valuse]

    def bpvector2floats(self, res):
        """
        Decode a vector string for blueprint.

        Args:
            res (str): The response string.

        Returns:
            list: The list of vectors.
        """
        values = re.findall(r'([XYZ]=\d+\.\d+)', res)
        return [[float(i) for i in value] for value in values]

    def decode_vertex(self, res):
        """
        Decode vertex locations.

        Args:
            res (str): Multi-line string of vertex coordinates.

        Returns:
            list[list[float]]: List of vertex coordinates [x, y, z].

        Example:
            >>> vertices = decoder.decode_vertex("0.0 0.0 0.0\\n1.0 1.0 1.0")
            >>> print(vertices)  # [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]]
        """
        lines = res.split('\n')
        lines = [line.strip() for line in lines]
        vertex_locations = [list(map(float, line.split())) for line in lines]
        return vertex_locations

    def decode_img(self, res, mode, inverse=False):
        """
        Decode an image.

        Args:
            res (str): The response string.
            mode (str): The image format (e.g., 'png', 'bmp', 'npy').
            inverse (bool): Whether to inverse the depth. Default is False.
        Returns:
            np.ndarray: The decoded image.
        Note: The depth image should use the 'npy' mode to decode.
        """
        if mode == 'png':
            img = self.decode_png(res)
        if mode == 'bmp':
            img = self.decode_bmp(res)
        if mode == 'npy':
            img = self.decode_depth(res, inverse)
        return img

    def decode_png(self, res):
        """
        Decode a PNG image.

        Args:
            res (str): The response string.

        Returns:
            np.ndarray: The decoded image.
        """
        img = np.asarray(PIL.Image.open(BytesIO(res)))
        img = img[:, :, :-1]  # delete alpha channel
        img = img[:, :, ::-1]  # transpose channel order
        return img

    def decode_bmp(self, res):
        """Decode BMP image data.

        Args:
            res (bytes): Raw BMP image data.

        Returns:
            np.ndarray: RGB image array of shape (H, W, 3) in uint8 format.

        Raises:
            ValueError: If image decoding fails.
        """
        nparr = np.frombuffer(res, np.uint8)
        
        # Decode image using OpenCV
        img = cv2.imdecode(nparr, cv2.IMREAD_UNCHANGED)
        
        if img is None:
            raise ValueError("Failed to decode image data")
        
        if len(img.shape) == 3 and img.shape[2] >= 3:
            img = img[:, :, :3]  # Remove alpha channel if present

        return img

    def decode_npy(self, res):
        """
        Decode a NPY image.

        Args:
            res (str): The response string.

        Returns:
            np.ndarray: The decoded image.
        """
        img = np.load(BytesIO(res))
        if len(img.shape) == 2:
            img = np.expand_dims(img, axis=-1)
        return img

    def decode_depth(self, res, inverse=False):
        """
        Decode a depth image.

        Args:
            res (str): The response string.
            inverse (bool): Whether to inverse the depth. Default is False.
        Returns:
            np.ndarray: The decoded depth image.
        """
        depth = np.load(BytesIO(res))
        if inverse:
            depth = 1/depth
        return np.expand_dims(depth, axis=-1)

    def empty(self, res):
        """
        Return the response as is.

        Args:
            res (str): The response string.

        Returns:
            str: The response string.
        """
        return res

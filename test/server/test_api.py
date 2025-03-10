import re
import time
from unrealcv.api import UnrealCv_API
from unrealcv.launcher import RunUnreal
from unrealcv.util import measure_fps, parse_resolution
import argparse
'''
An example to show how to use the UnrealCV API to launch the game and run some functions
'''


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--env-bin', default='UE5_ExampleScene_Win64\\UE5_ExampleScene_Win64\\Compile_unrealcv5_4\\Binaries\\UE5_ExampleScene_Win64\\Compile_unrealcv5_4.exe', help='The path to the UE4Editor binary')
    parser.add_argument('--env-map', default='Old_Town', help='The map to load')
    parser.add_argument('--use-docker', action='store_true', help='Run the game in a docker container')
    parser.add_argument('--resolution', '-res', default='640x480', help='The resolution in the unrealcv.ini file')
    parser.add_argument('--display', default=None, help='The display to use')
    parser.add_argument('--use-opengl', action='store_true', help='Use OpenGL for rendering')
    parser.add_argument('--offscreen', action='store_true', help='Use offscreen rendering')
    parser.add_argument('--nullrhi', action='store_true', help='Use the NullRHI')
    parser.add_argument('--show', action='store_true', help='show the get image result')
    parser.add_argument('--gpu-id', default=None, help='The GPU to use')
    parser.add_argument('--editor', action='store_true', help='direct connect to the env without starting the game')
    parser.add_argument('--port', default=9000, help='env port')
    args = parser.parse_args()

    if args.editor:
        env_ip = '127.0.0.1'
        env_port = int(args.port)
    else:
        env_bin = args.env_bin
        env_map = args.env_map
        # config the game binary and map
        ue_binary = RunUnreal(ENV_BIN=env_bin, ENV_MAP=env_map)
        # start the game
        env_ip, env_port = ue_binary.start(args.use_docker, parse_resolution(args.resolution), args.display, args.use_opengl, args.offscreen, args.nullrhi, str(args.gpu_id))

    # connect to the game
    unrealcv = UnrealCv_API(env_port, env_ip, parse_resolution(args.resolution), 'tcp')  # 'tcp' or 'unix', 'unix' is only for local machine in Linux
    unrealcv.set_map(env_map)

    # Test the API
    print(unrealcv.get_camera_num())
    print(unrealcv.camera_info())
    objects = unrealcv.get_objects()
    t = time.time()
    unrealcv.build_color_dict(objects, batch=True)
    print(time.time() - t)

    # unrealcv.get_obj_bboxes(unrealcv.get_image(1, 'seg'), objects)
    print(unrealcv.get_obj_pose(objects[0]))
    print(unrealcv.get_obj_location(objects[0]))
    print(unrealcv.get_obj_rotation(objects[0]))
    # images = unrealcv.get_image_multicam(range(unrealcv.get_camera_num()))
    for cam_id in range(unrealcv.get_camera_num()):
        for viewmode in ['lit', 'normal', 'seg', 'depth']:
            for mode in ['bmp', 'png']:
                if viewmode == 'depth' and mode == 'png':
                    img = unrealcv.get_depth(cam_id, inverse=True, show=True)
                else:
                    img = unrealcv.get_image(cam_id, viewmode, mode, show=True, inverse=True)
                fps = measure_fps(unrealcv.get_image, cam_id, viewmode, mode, show=args.show)
                print(f'FPS for cam {cam_id}, mode {viewmode}, mode {mode}: {fps}')


    unrealcv.client.disconnect()
    if not args.editor:
        ue_binary.close()


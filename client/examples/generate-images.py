import sys
sys.path.append('..')
from unrealcv import client
from ipynb_util import read_camera_info, render_frame

if __name__ == '__main__':
    client.connect()

    if not client.isconnected():
        print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
    else:
        camera_pos = read_camera_info('./realistic_rendering_camera_info.txt')

        frames = []
        for frame_index in range(len(camera_pos)):
            pos = camera_pos[frame_index]
            f = render_frame(client, pos)
            print f
            frames.append(f)

import sys
sys.path.append('..')
import ue4cv
from ipynb_util import read_camera_info, render_frame

if __name__ == '__main__':
    ue4cv.client.connect()

    if not ue4cv.client.isconnected():
        print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
    else:
        camera_pos = read_camera_info('./realistic_rendering_camera_info.txt')

        frames = []
        for frame_index in range(len(camera_pos)):
            pos = camera_pos[frame_index]
            f = render_frame(ue4cv.client, pos)
            print f
            frames.append(f)

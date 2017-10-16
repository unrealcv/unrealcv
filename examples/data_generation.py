from unrealcv import client
from unrealcv.automation import UE4Binary
from unrealcv.util import read_png, read_npy
# If you see errors of unrealcv.automation and util, use pip install --upgrade unrealcv

import matplotlib.pyplot as plt
import numpy as np
import json

class PlaybackSequence:
    def __init__(self, json_filename):
        record = json.load(open(json_filename))
        self.record = record
        self.scene_name = record['scene_name']
        self.cam_info = record['cam_info']

    def get(self, frameid):
        frame = self.cam_info[frameid]
        loc = dict(x = frame['x'], y = frame['y'], z = frame['z'])
        rot = dict(pitch = frame['pitch'], yaw = frame['yaw'], roll = frame['roll'])
        return loc, rot

    def __len__(self):
        return len(self.cam_info)

class Camera:
    def __init__(self, client, camera_id):
        self.id = camera_id
        self.client = client

    def set_location(self, loc):
        self.client.request('vset /camera/{id}/location {x} {y} {z}'.format(id = self.id, **loc))

    def set_rotation(self, rot):
        self.client.request('vset /camera/{id}/rotation {pitch} {yaw} {roll}'.format(id = self.id, **rot))

    def capture_depth(self):
        res = self.client.request('vget /camera/{id}/depth npy'.format(id = self.id))
        depth = read_npy(res)
        return depth

    def capture_img(self):
        res = self.client.request('vget /camera/{id}/lit png'.format(id = self.id))
        img = read_png(res)
        return img

def main():
    binary_path = r'C:\qiuwch\workspace\unrealcv\Binaries\RealisticRendering.uproject\WindowsNoEditor\RealisticRendering.exe'
    binary = UE4Binary(binary_path)
    playback_seq = PlaybackSequence('./rr_573.json')
    camera = Camera(client, 0)

    with binary:
        client.connect()
        # res = client.request('vget /camera/0/lit rr.png')
        # print(res)
        #
        # res = client.request('vget /camera/0/lit png')
        # img = read_png(res)
        # plt.imsave('lit.png', img)

        for frameid in range(len(playback_seq)):
            print('Capture frame %d' % frameid)
            [loc, rot] = playback_seq.get(frameid)
            camera.set_location(loc)
            camera.set_rotation(rot)
            img = camera.capture_img()
            depth = camera.capture_depth()
        # Save to disk

if __name__ == '__main__':
    main()

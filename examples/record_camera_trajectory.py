import sys, atexit, argparse, json, time
sys.path.append('..')
from unrealcv import client

trajectory = []

def message_handler(message):
    if message == 'clicked':
        rot = [float(v) for v in client.request('vget /camera/0/rotation').split(' ')]
        loc = [float(v) for v in client.request('vget /camera/0/location').split(' ')]
        trajectory.append(dict(rotation = rot, location = loc))

def save_to_file(filename):
    if len(trajectory) != 0:
        with open(filename, 'w') as f:
            json.dump(trajectory, f, indent = 4)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--filename', default='camera-trajectory.json')
    args = parser.parse_args()

    atexit.register(save_to_file, args.filename)
    client.connect()
    client.message_handler = message_handler
    if not client.isconnected():
        print 'Can not connect to the game, please run the game downloaded from http://unrealcv.github.io first'
    else:
        time.sleep(60 * 60 * 24)

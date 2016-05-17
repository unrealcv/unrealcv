from common_config import *
import argparse, time


def render_frame(client, pos):
    loc = pos[0] # location
    rot = pos[1] # rotation
    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok'
    time.sleep(2)
    files = []
    cmd = 'vget /camera/0/image'
    # TODO: need a way to check error
    files.append(client.request(cmd))
    cmd = 'vget /camera/0/depth'
    files.append(client.request(cmd))
    cmd = 'vget /camera/0/object_mask'
    files.append(client.request(cmd))
    return files

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('logfile')
    args = parser.parse_args()

    with open(args.logfile, 'r') as f:
        lines = f.readlines()
    # Parse camera location and rotation from file
    camera_pos = []
    for line_id in range(len(lines)):
        line = lines[line_id].strip() # Remove \n at the end
        if line_id % 3 == 0: # filename
            pass
        elif line_id % 3 == 1: # location
            location = [float(v) for v in line.split(' ')]
        elif line_id % 3 == 2: # Rotation
            rotation = [float(v) for v in line.split(' ')]
            camera_pos.append((location, rotation))

    client = Client((HOST, PORT), None)
    all_files = []
    for pos in camera_pos:
        all_files.append(render_frame(client, pos))

    print all_files
    with open('testlog.txt', 'w') as f:
        f.write(all_files)


    #
    # time.sleep(60 * 60)

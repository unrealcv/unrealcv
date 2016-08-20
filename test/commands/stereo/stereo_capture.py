import time, sys, shutil
sys.path.append('../../..')
# Use timer to check time.
import ue4cv
import time, math, os

ue4cv.client.connect()

class FPSCounter:
    def __init__(self):
        self.start_index = 0
        self.start_time = time.time()

    def tick(self, current_index):
        current_time = time.time()
        if (current_time - self.start_time > 1):
            print '%d: %d fps' % (current_index, (current_index - self.start_index))
            self.start_index = current_index
            self.start_time = current_time

counter = FPSCounter()

with open('cam_seq.csv', 'r') as f:
    lines = f.readlines()
    lines = lines[1:]
    seqs = [[float(v) for v in line.split(',')] for line in lines]

left_eye_folder = 'left'
right_eye_folder = 'right'
for folder in [left_eye_folder, right_eye_folder]:
    if not os.path.isdir(folder):
        os.mkdir(folder)

for (id, x, y, z, pitch, yaw) in seqs:
    suffix = ''
    if not (math.isnan(x) or math.isnan(y) or math.isnan(z)):
        res = ue4cv.client.request('vset /camera/0/location %f %f %f' % (x, y, z))
        # res = ue4cv.client.request('vset /camera/0/moveto %f %f %f' % (x, y, z))
        suffix += '_loc'
    if not (math.isnan(pitch) or math.isnan(yaw)):
        res = ue4cv.client.request('vset /camera/0/rotation %f %f 0' % (pitch, yaw))
        suffix += '_rot'
    left_filename = ue4cv.client.request('vget /camera/0/lit')
    right_filename = ue4cv.client.request('vget /camera/1/lit')
    # copy file
    shutil.copyfile(left_filename, os.path.join(left_eye_folder, '%06d%s_l.png') % (id, suffix))
    shutil.copyfile(right_filename, os.path.join(right_eye_folder, '%06d%s_r.png') % (id, suffix))
    counter.tick(id)

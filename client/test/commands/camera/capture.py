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

def run_capture(delay, output_folder, cmd):
    if not os.path.isdir(output_folder):
        os.mkdir(output_folder)

    with open('cam_seq.csv', 'r') as f:
        lines = f.readlines()
        lines = lines[1:]
        seqs = [[float(v) for v in line.split(',')] for line in lines]

    counter = FPSCounter()
    for (id, x, y, z, pitch, yaw) in seqs:
        suffix = ''
        if not (math.isnan(x) or math.isnan(y) or math.isnan(z)):
            res = ue4cv.client.request('vset /camera/0/location %f %f %f' % (x, y, z))
            # res = ue4cv.client.request('vset /camera/0/moveto %f %f %f' % (x, y, z))
            suffix += '_loc'
        if not (math.isnan(pitch) or math.isnan(yaw)):
            res = ue4cv.client.request('vset /camera/0/rotation %f %f 0' % (pitch, yaw))
            suffix += '_rot'
        time.sleep(delay)
        filename = ue4cv.client.request(cmd)
        # copy file
        shutil.copyfile(filename, os.path.join(output_folder, '%06d%s.png') % (id, suffix))
        counter.tick(id)

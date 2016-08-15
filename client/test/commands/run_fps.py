# Test fps of UnrealCV.

import time, sys
sys.path.append('../..')
# Use timer to check time.
import ue4cv

ue4cv.client.connect()

def echo():
    res = ue4cv.client.request('vget /unrealcv/echo hello')
    # res = ue4cv.client.request('vget /camera/0/lit')
    # res = ue4cv.client.request('vget /camera/0/depth depth.exr')

class FPSCounter:
    def __init__(self):
        self.start_index = 0
        self.start_time = time.time()

    def tick(self, current_index):
        current_time = time.time()
        if (current_time - self.start_time > 1):
            print '%d fps' % (current_index - self.start_index)
            self.start_index = current_index
            self.start_time = current_time

def main():
    counter = FPSCounter()
    n_iter = 1000000
    for i in range(n_iter):
        counter.tick(i)
        echo()

    time.sleep(5)

if __name__ == '__main__':
    main()

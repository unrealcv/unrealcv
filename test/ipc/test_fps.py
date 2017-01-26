import unittest, time, random, argparse

import sys; sys.path.append('..')
from testcfg import client

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

def test_move():
    counter = FPSCounter()
    loc = client.request('vget /camera/0/location').split(' ')
    rot = client.request('vget /camera/0/rotation').split(' ')

    n_iter = 100
    print 'Run command for %d iteration, will take some time' % n_iter
    for i in range(n_iter):
        counter.tick(i)
        jitter = [random.random() * 10 for _ in range(3)]
        loc = [str(float(a)+b) for (a,b) in zip(loc, jitter)]
        rot = [str(float(a)+b) for (a,b) in zip(rot, jitter)]

        res = client.request('vset /camera/0/location %s' % ' '.join(loc))
        res = client.request('vset /camera/0/rotation %s' % ' '.join(rot))

def test_lit():
    counter = FPSCounter()
    loc = client.request('vget /camera/0/location').split(' ')
    rot = client.request('vget /camera/0/rotation').split(' ')

    n_iter = 100
    print 'Run command for %d iteration, will take some time' % n_iter
    for i in range(n_iter):
        counter.tick(i)
        jitter = [random.random() * 10 for _ in range(3)]
        loc = [str(float(a)+b) for (a,b) in zip(loc, jitter)]
        rot = [str(float(a)+b) for (a,b) in zip(rot, jitter)]

        res = client.request('vset /camera/0/location %s' % ' '.join(loc))
        res = client.request('vset /camera/0/rotation %s' % ' '.join(rot))
        res = client.request('vget /camera/0/lit')

def test_depth():
    counter = FPSCounter()

    n_iter = 100
    print 'Run command for %d iteration, will take some time' % n_iter
    for i in range(n_iter):
        counter.tick(i)
        jitter = [random.random() * 10 for _ in range(3)]
        loc = [str(float(a)+b) for (a,b) in zip(loc, jitter)]
        rot = [str(float(a)+b) for (a,b) in zip(rot, jitter)]

        res = client.request('vset /camera/0/location %s' % ' '.join(loc))
        res = client.request('vset /camera/0/rotation %s' % ' '.join(rot))
        res = client.request('vget /camera/0/lit')

def test_sync():
    counter = FPSCounter()
    n_iter = 100
    for i in range(n_iter):
        counter.tick(i)
        res = client.request('vget /unrealcv/sync %s' % 'hi')

def test_async():
    counter = FPSCounter()
    n_iter = 100
    for i in range(n_iter):
        counter.tick(i)
        res = client.request('vset /unrealcv/async %s' % 'hi')


# test_suite = unittest.TestLoader().loadTestsFromTestCase(FPSTestCase)
if __name__ == '__main__':
    # unittest.main()
    client.connect()
    # test_sync()
    # test_lit()
    # test_move()
    parser = argparse.ArgumentParser()
    parser.add_argument('--sync', action='store_true')
    parser.add_argument('--async', action='store_true')
    parser.add_argument('--move', action='store_true')
    parser.add_argument('--lit', action='store_true')
    parser.add_argument('--depth', action='store_true')
    args = parser.parse_args()
    if args.sync:
        test_sync()
    if args.async:
        test_async()
    if args.move:
        test_move()
    if args.lit:
        test_lit()
    if args.depth:
        test_depth()

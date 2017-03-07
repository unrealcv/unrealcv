# pytest -s stereo.py
from unrealcv import client
import math, random

def isok(res):
    return res == 'ok'

class Vec3:
    def __init__(self, data):
        if isinstance(data, str):
            self.vec = [float(v) for v in data.split(' ')]
        if isinstance(data, list):
            self.vec = data

    def __str__(self):
        return ' '.join([str(v) for v in self.vec])

    def l2norm(self):
        return math.sqrt(sum([v*v for v in self.vec]))

    def __sub__(self, v):
        return Vec3([a-b for (a,b) in zip(self.vec, v.vec)])

    def __add__(self, v):
        return Vec3([a+b for (a,b) in zip(self.vec, v.vec)])

def random_vec3(min=-90, max=90):
    return Vec3([random.randrange(min, max) for _ in range(3)])

def approx(a, b, tol = 0.01):
    return abs(a - b) < tol

def test_vec3():
    a = Vec3([0, 0, 0])
    b = Vec3([1, 1, 1])
    assert(approx((a-b).l2norm(), math.sqrt(3)))
    assert(approx((a+b).l2norm(), math.sqrt(3)))

def test_camera_distance():
    isok(client.connect())

    for test_distance in [20, 40, 60]:
        res = client.request('vset /action/eyes_distance %d' % test_distance)
        assert isok(res)

        for _ in range(5):
            client.request('vset /camera/0/rotation %s' % str(random_vec3()))
            actor_loc = Vec3(client.request('vget /actor/location'))
            loc1 = Vec3(client.request('vget /camera/0/location'))
            loc2 = Vec3(client.request('vget /camera/1/location'))
            print actor_loc, loc1, loc2

            actual_dist = (loc1 - loc2).l2norm()
            expect_dist = test_distance
            assert approx(actual_dist, expect_dist)
            actor_cam0_distance = (actor_loc - loc1).l2norm()
            assert approx(actor_cam0_distance, 0)

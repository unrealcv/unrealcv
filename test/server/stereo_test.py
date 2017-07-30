# pytest -s stereo.py -k [name]
from unrealcv import client
import math, random
from conftest import checker, ver
import pytest

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

@pytest.mark.skipif(ver() < (0, 3, 2), reason = 'eyes_distance is implemented before v0.3.2')
def test_camera_distance():
    client.connect()

    for test_distance in [20, 40, 60]:
        res = client.request('vset /action/eyes_distance %d' % test_distance)
        assert checker.is_ok(res)

        for _ in range(5):
            client.request('vset /camera/0/rotation %s' % str(random_vec3()))
            actor_loc = Vec3(client.request('vget /actor/location'))
            loc1 = Vec3(client.request('vget /camera/0/location'))
            loc2 = Vec3(client.request('vget /camera/1/location'))
            print('%s %s %s' % (actor_loc, loc1, loc2))

            actual_dist = (loc1 - loc2).l2norm()
            expect_dist = test_distance
            assert approx(actual_dist, expect_dist)
            actor_cam0_distance = (actor_loc - loc1).l2norm()
            assert approx(actor_cam0_distance, 0)

@pytest.mark.skipif(ver() < (0, 3, 2), reason = 'pause is implemented before v0.3.2')
def test_pause():
    client.connect()
    cmds = [
        'vset /action/game/pause',
        'vget /camera/0/lit',
        'vget /camera/1/lit',
        'vset /action/game/pause',
    ]
    for cmd in cmds:
        res = client.request(cmd)
        assert checker.not_error(res)

if __name__ == '__main__':
    def test_vec3():
        a = Vec3([0, 0, 0])
        b = Vec3([1, 1, 1])
        assert(approx((a-b).l2norm(), math.sqrt(3)))
        assert(approx((a+b).l2norm(), math.sqrt(3)))

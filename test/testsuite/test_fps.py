import unittest, time, random
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

class FPSTestCase(unittest.TestCase):
    def setUp(self):
        client.connect()

    def test_get_lit(self):
        counter = FPSCounter()
        loc = client.request('vget /camera/0/location').split(' ')
        rot = client.request('vget /camera/0/rotation').split(' ')

        n_iter = 1000000
        for i in range(n_iter):
            counter.tick(i)
            jitter = [random.random() * 10 for _ in range(3)]
            loc = [str(float(a)+b) for (a,b) in zip(loc, jitter)]
            rot = [str(float(a)+b) for (a,b) in zip(rot, jitter)]

            res = client.request('vset /camera/0/location %s' % ' '.join(loc))
            res = client.request('vset /camera/0/rotation %s' % ' '.join(rot))
            res = client.request('vget /camera/0/lit')

test_suite = unittest.TestLoader().loadTestsFromTestCase(FPSTestCase)
if __name__ == '__main__':
    # unittest.main()

    suite = unittest.TestSuite()
    suite.addTest(FPSTestCase("test_get_lit"))
    unittest.TextTestRunner(verbosity=2).run(suite)

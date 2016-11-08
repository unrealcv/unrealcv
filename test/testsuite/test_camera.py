import unittest, logging, os, time
import imageio
logging.basicConfig(verbosity=logging.debug)
logger = logging.getLogger(__name__)

from testcfg import client

'''
Define a set of tests for 'vget /camera/*' and 'vset /camera/*'
'''

def request(cmd):
    res = client.request(cmd)
    logger.info(cmd)
    logger.info(res)
    return res

class CameraControlTestCase(unittest.TestCase):
    # Test getting and setting camera location and rotation
    def setUp(self):
        client.connect()

    def test_init_location(self):
        # For playground project, we know the initial position, so we can do assert here
        pass

    def test_location(self):
        # Set location and get location
        res = request('vget /camera/0/location')
        loc = [float(v) for v in res.split(' ')]
        inLoc = dict(x = loc[0] + 1, y = loc[1] + 2, z = loc[2] + 3) # Random preturb location

        res = client.request('vset /camera/0/location {x} {y} {z}'.format(**inLoc))
        self.assertEqual(res, 'ok')

        res = client.request('vget /camera/0/location')
        outLoc = [float(v) for v in res.split(' ')]
        self.assertEqual(inLoc['x'], outLoc[0])
        self.assertEqual(inLoc['y'], outLoc[1])
        self.assertEqual(inLoc['z'], outLoc[2])

    def test_rotation(self):
        res = request('vget /camera/0/rotation')
        rot = [float(v) for v in res.split(' ')]
        inRot = dict(pitch = rot[0] + 1, yaw = rot[1] + 2, roll = rot[2] + 3)

        res = request('vset /camera/0/rotation {pitch} {yaw} {roll}'.format(**inRot))
        self.assertEqual(res, 'ok')

        res = request('vget /camera/0/rotation')
        outRot = [float(v) for v in res.split(' ')]
        self.assertEqual(inRot['pitch'], outRot[0])
        self.assertEqual(inRot['yaw'], outRot[1])
        self.assertEqual(inRot['roll'], outRot[2])

class GroundTruthTestCase(unittest.TestCase):
    def setUp(self):
        client.connect()

    def is_justcreated(self, filename):
        now = time.time()
        t = os.path.getctime(filename)
        delta = now - t
        timelimit = 5
        self.assertTrue(delta > 0) # file is created before this function
        self.assertTrue(delta < timelimit) # file is just created

    def is_png(self, filename):
        filename = filename.replace('D:/', '/mnt/d/') # This is a temporary hack
        logger.warning(filename)
        self.assertTrue(filename == filename.strip())
        self.assertTrue(filename != '')
        self.assertTrue(filename.endswith('.png'))
        self.assertTrue(os.path.isfile(filename))

        # Make sure file is just created
        self.is_justcreated(filename)

    def is_exr(self, filename):
        filename = filename.replace('D:/', '/mnt/d/') # This is a temporary hack
        logger.warning(filename)
        self.assertTrue(filename == filename.strip())
        self.assertTrue(filename != '')
        self.assertTrue(filename.endswith('.exr'))
        self.assertTrue(os.path.isfile(filename))

        # Make sure file is just created
        self.is_justcreated(filename)
        depth = imageio.imread(filename)
        print depth.shape

    def test_camera(self):
        tasks = []
        viewmodes = ['normal', 'depth', 'lit', 'object_mask']
        for v in viewmodes:
            res = request('vget /camera/0/%s' % v)
            self.is_png(res)

    def test_exr_depth(self):
        res = request('vget /camera/0/depth depth.exr')
        self.is_exr(res)

class ObjTypeTestCase(unittest.TestCase):
    # Make sure all types of objects can be supported
    def test_blueprint(self):
        pass

    def test_landscape(self):
        pass

    def test_static(self):
        pass


camera_control = unittest.TestLoader().loadTestsFromTestCase(CameraControlTestCase)
ground_truth = unittest.TestLoader().loadTestsFromTestCase(GroundTruthTestCase)

test_suite = unittest.TestSuite([camera_control, ground_truth])
if __name__ == '__main__':
    unittest.TextTestRunner(verbosity=2).run(test_suite)

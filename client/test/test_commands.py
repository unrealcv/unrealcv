import unittest, random
from common_conf import *
from checker import *
import ue4cv
_L = logging.getLogger(__name__)
_L.setLevel(logging.DEBUG)
# _L.addHandler(logging.StreamHandler())

request = ue4cv.client.request

def read_camera_info(filename):
    with open(filename) as f:
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
    return camera_pos

class TestCommands(unittest.TestCase):
    # Test a generic small game
    @classmethod
    def setUpClass(cls):
        ue4cv.client.connect()
        cls.camera_poses = read_camera_info('./correctness_test/camera_info_basename.txt')

    def test_objects(self):
        response = request('vget /objects')
        # _L.debug(response)
        self.assertTrue(validate_format(response))

        objects = response.split(' ')[:10]
        self.assertTrue(len(objects) > 0)

        tasks = []
        for objname in objects:
            tasks.append(['vget /object/%s/name' % objname, objname])
            tasks.append(['vget /object/%s/color' % objname, skip])
            # TODO: add a function to check regular expression

        run_tasks(self, ue4cv.client, tasks)

    def test_set_location(self):
        for pos in self.camera_poses:
            loc = pos[0] # location
            param_str = '%.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
            cmd = 'vset /camera/0/location %s' % param_str
            response = request(cmd)
            self.assertEqual(response, 'ok')
            response = request('vget /camera/0/location')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

        # TODO: also try random positions
        # Will there be some invalid location that can not be set to?
        # Can I know the boundary of the world?
        for _ in range(100):
            param_str = '%.3f %.3f %.3f' % (random.randrange(100), random.randrange(100), random.randrange(100))
            cmd = 'vset /camera/0/location %s' % param_str
            response = request(cmd)
            self.assertEqual(response, 'ok')
            response = request('vget /camera/0/location')
            print 'expect %s, got %s' % (param_str, response)

    # Try to reach the limit of the scene
    def test_location_limit(self):
        for pos in self.camera_poses:
            loc = pos[0] # location
            param_str = '%.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
            cmd = 'vset /camera/0/location %s' % param_str
            response = request(cmd)
            self.assertEqual(response, 'ok')
            response = request('vget /camera/0/location')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

        # TODO: also try random positions
        # Will there be some invalid location that can not be set to?
        # Can I know the boundary of the world?
        for _ in range(100):
            param_str = '%.3f %.3f %.3f' % (random.randrange(100), random.randrange(100), random.randrange(100))
            cmd = 'vset /camera/0/location %s' % param_str
            response = request(cmd)
            self.assertEqual(response, 'ok')
            response = request('vget /camera/0/location')
            print 'expect %s, got %s' % (param_str, response)


    def test_set_rotation(self):
        for pos in self.camera_poses:
            rot = pos[1] # rotation
            param_str = '%.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
            cmd = 'vset /camera/0/rotation %s' % param_str
            response = request(cmd)
            self.assertEqual(response, 'ok')
            response = request('vget /camera/0/rotation')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

    def test_camera(self):
        tasks = []
        modes = ['normal', 'base_color', 'depth', 'lit', 'unlit', 'view', 'object_mask']
        for mode in modes:
            tasks.append(['vget /camera/0/%s' % mode, ispng])

        run_tasks(self, ue4cv.client, tasks)

    def test_viewmode(self):
        tasks = []
        modes = ['depth', 'lit', 'unlit', 'normal', 'object_mask']
        for mode in modes:
            tasks.append(['vset /mode/%s' % mode, 'ok']) # TODO: Change it to vset /mode modename
            tasks.append(['vget /mode', mode])

        run_tasks(self, ue4cv.client, tasks)

if __name__ == '__main__':
    logging.basicConfig()
    # unittest.main(verbosity = 2)

    tests = [
        # 'test_camera',
        # 'test_viewmode',
        # 'test_objects',
        # 'test_set_rotation',
        # 'test_set_location',
        'test_location_limit',
    ]
    suite = unittest.TestSuite(map(TestCommands, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

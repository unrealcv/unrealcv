import unittest, random, logging
from test_cfg import client
import testutil
_L = logging.getLogger(__name__)
_L.setLevel(logging.DEBUG)
# _L.addHandler(logging.StreamHandler())

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
    host = 'localhost'
    port = 9000
    # Test a generic small game
    @classmethod
    def setUpClass(cls):
        client.connect()
        if not client.isconnected():
            raise Exception('Can not connect to a running game instance')
        # cls.camera_poses = read_camera_info('./correctness_test/camera_info_basename.txt')

    def test_objects(self):
        response = client.request('vget /objects')
        # _L.debug(response)
        self.assertTrue(testutil.validate_format(response))

        objects = response.split(' ')[:10]
        self.assertTrue(len(objects) > 0)

        tasks = []
        for objname in objects:
            tasks.append(['vget /object/%s/name' % objname, objname])
            tasks.append(['vget /object/%s/color' % objname, testutil.skip])
            # TODO: add a function to check regular expression

        testutil.run_tasks(self, client, tasks)

    def test_set_location(self):
        for pos in self.camera_poses:
            loc = pos[0] # location
            param_str = '%.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
            cmd = 'vset /camera/0/location %s' % param_str
            response = client.request(cmd)
            self.assertEqual(response, 'ok')
            response = client.request('vget /camera/0/location')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

        # TODO: also try random positions
        # Will there be some invalid location that can not be set to?
        # Can I know the boundary of the world?
        for _ in range(5):
            param_str = '%.3f %.3f %.3f' % (random.randrange(100), random.randrange(100), random.randrange(100))
            cmd = 'vset /camera/0/location %s' % param_str
            response = client.request(cmd)
            self.assertEqual(response, 'ok')
            response = client.request('vget /camera/0/location')
            print 'expect %s, got %s' % (param_str, response)

    # Try to reach the limit of the scene
    def test_location_limit(self):
        for pos in self.camera_poses:
            loc = pos[0] # location
            param_str = '%.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
            cmd = 'vset /camera/0/location %s' % param_str
            response = client.request(cmd)
            self.assertEqual(response, 'ok')
            response = client.request('vget /camera/0/location')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

        # TODO: also try random positions
        # Will there be some invalid location that can not be set to?
        # Can I know the boundary of the world?
        for _ in range(5):
            param_str = '%.3f %.3f %.3f' % (random.randrange(100), random.randrange(100), random.randrange(100))
            cmd = 'vset /camera/0/location %s' % param_str
            response = client.request(cmd)
            self.assertEqual(response, 'ok')
            response = client.request('vget /camera/0/location')
            print 'expect %s, got %s' % (param_str, response)


    def test_set_rotation(self):
        for pos in self.camera_poses:
            rot = pos[1] # rotation
            param_str = '%.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
            cmd = 'vset /camera/0/rotation %s' % param_str
            response = client.request(cmd)
            self.assertEqual(response, 'ok')
            response = client.request('vget /camera/0/rotation')
            self.assertEqual(response, param_str, 'expect %s, got %s' % (param_str, response))

    def test_camera(self):
        tasks = []
        modes = ['normal', 'depth', 'lit', 'object_mask']
        # base_color, unlit
        for mode in modes:
            tasks.append(['vget /camera/0/%s' % mode, testutil.ispng])

        testutil.run_tasks(self, client, tasks)

    def test_viewmode(self):
        tasks = []
        modes = ['depth', 'lit', 'unlit', 'normal', 'object_mask']
        for mode in modes:
            tasks.append(['vset /viewmode %s' % mode, 'ok'])
            tasks.append(['vget /viewmode', mode])

        testutil.run_tasks(self, client, tasks)

    # def test_set_port(self):
    #     client.disconnect()
    #
    #     client = ue4cv.Client((self.host, 9000))
    #     client.connect()
    #     self.assertEqual(client.isconnected(), True)
    #     response = client.request('vget /unrealcv/port')
    #     self.assertEqual(response, '9000')
    #     response = client.request('vset /unrealcv/port 9001', timeout=1) # This is a special command and should not expect a response
    #     self.assertEqual(response, None)
    #     self.assertEqual(client.isconnected(), False)
    #
    #     client = ue4cv.Client((self.host, 9001))
    #     client.connect()
    #     self.assertEqual(client.isconnected(), True)
    #     response = client.request('vget /unrealcv/port')
    #     self.assertEqual(response, '9001')
    #     response = client.request('vset /unrealcv/port 9000', timeout=1) # This is a special command and should not expect a response
    #     self.assertEqual(response, None)
    #     self.assertEqual(client.isconnected(), False)
    #
    #     ue4cv.client.connect()

if __name__ == '__main__':
    logging.basicConfig()
    # unittest.main(verbosity = 2)

    tests = [
        'test_camera',
        'test_viewmode',
        'test_objects',
        # 'test_set_rotation',
        # 'test_set_location',
        # 'test_location_limit',
        # 'test_set_port', # Not stable yet
    ]
    suite = unittest.TestSuite(map(TestCommands, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

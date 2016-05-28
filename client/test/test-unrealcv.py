import unittest, time
from checker import *
from test_common import *
import argparse

client = Client((HOST, PORT), None) # Try different port number
in_develop_mode = True

class TestCommands(unittest.TestCase):
    # TODO: Test some error handling case
    def setUp(self): # this setUp will be run for each test function??
        pass
        # self.client = Client((HOST, PORT), None) # Try different port number

    def run_tasks(self, tasks):
        for task in tasks:
            cmd = task[0]
            expect = task[1]

            print 'Cmd: %s' % cmd
            response = client.request(cmd)
            print 'Response: %s' % repr(response)
            # Need to lock until I got a reply
            # print reply

            if isinstance(expect, str):
                self.assertEqual(response, expect, 'Expect %s, Response %s' % ( expect, response))
            else:
                self.assertTrue(expect(response))

    @unittest.skipIf(in_develop_mode)
    def test_objects(self):
        response = client.request('vget /objects')
        response = response.strip() # TODO: remove this line
        self.assertTrue(validate_format(response))

        objects = response.split(' ')
        self.assertTrue(len(objects) > 0)

        print 'Number of objects %d' % len(objects)
        tasks = []
        for objname in objects:
            tasks.append(['vget /object/%s/name' % objname, objname])
            tasks.append(['vget /object/%s/color' % objname, skip])
            # TODO: add a function to check regular expression

        self.run_tasks(tasks)

    # @unittest.expectedFailure # TODO: Need to fix location
    @unittest.skipIf(in_develop_mode)
    def test_camera(self):
        tasks = [
            ['vget /camera/0/location', skip],
            ['vset /camera/0/rotation 0.0 0.0 0.0', 'ok'],
            ['vget /camera/0/rotation', '0.000 0.000 0.000'],
            # TODO: Change the Uri handle to exact map
            # The regexp in server needs to do exact match
            ['vset /camera/0/location 0.0 0.0 0.0', 'ok'],
            ['vget /camera/0/location', '0.000 0.000 0.000'],
        ]


        modes = ['normal', 'base_color', 'depth', 'lit', 'unlit', 'view', 'object_mask']
        for mode in modes:
            tasks.append(['vget /camera/0/%s' % mode, ispng])

        self.run_tasks(tasks)

    @unittest.skip(in_develop_mode)
    def test_viewmode(self):
        tasks = []
        modes = ['depth', 'lit', 'unlit', 'normal', 'object_mask']
        for mode in modes:
            tasks.append(['vset /mode/%s' % mode, 'ok']) # TODO: Change it to vset /mode modename
            tasks.append(['vget /mode', mode])

        self.run_tasks(tasks)


class TestNetworkConnection(unittest.TestCase):
    def test_timeout(self):
        # Check whether the request timeout can work well.
        pass

    def test_disconnected(self):
        pass

    @unittest.skipIf(in_develop_mode)
    def test_double_connection(self):
        print 'Testing double connection'
        second_client = Client((HOST, PORT), None)
        second_client.request('vget /mode')

if __name__ == '__main__':
    # parser = argparse.ArgumentParser()
    # parser.add_argument('--develop', action='store_true')
    #
    # args = parser.parse_args()
    #
    # in_develop_mode = args.develop
    # if '--develop' in sys.argv:
    #     sys.argv.remove('--develop')
    parser = argparse.ArgumentParser()
    parser.add_argument('--develop', action='store_true')
    args = parser.parse_args()
    in_develop_mode = args.develop
    print(in_develop_mode)

    unittest.main()

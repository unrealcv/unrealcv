import unittest, time, sys
from checker import *
from test_common import *
from test_server import EchoServer, MessageServer

if '--develop' in sys.argv:
    in_develop_mode = True
    sys.argv.remove('--develop')
else:
    in_develop_mode = False

# client = Client((HOST, PORT), None) # Try different port number
# in_develop_mode = False


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

    @unittest.skipIf(in_develop_mode, 'skip')
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
    @unittest.skipIf(in_develop_mode, 'skip')
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

    @unittest.skipIf(in_develop_mode, 'skip')
    def test_viewmode(self):
        tasks = []
        modes = ['depth', 'lit', 'unlit', 'normal', 'object_mask']
        for mode in modes:
            tasks.append(['vset /mode/%s' % mode, 'ok']) # TODO: Change it to vset /mode modename
            tasks.append(['vget /mode', mode])

        self.run_tasks(tasks)


def start_echo_server():
    def _():
        server = EchoServer((HOST, PORT))
        server.start()

    import threading
    receiving_thread = threading.Thread(target = _)
    receiving_thread.setDaemon(1)
    receiving_thread.start() # TODO: stop this thread
    time.sleep(0.1) # Wait for the server started
    print 'Started'


class TestBaseClient(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.server = EchoServer((HOST, PORT))
        cls.server.start()
        cls.base_client = BaseClient((HOST, PORT), None)

    @classmethod
    def tearDownClass(cls):
        print 'tear down'
        cls.server.shutdown()

    def test_send(self):
        msg = "Hello"
        client1 = BaseClient((HOST, PORT), None)
        client2 = BaseClient((HOST, PORT), None)
        client3 = BaseClient((HOST, PORT), None)
        client4 = BaseClient((HOST, PORT), None)
        for client in [client1, client2, client3, client4]:
            self.assertEqual(self.base_client.send(msg), True, 'Can not send message')


class TestNetworkConnection(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        # Start a test server
        start_echo_server()

    # def test_timeout(self):
    #     # Check whether the request timeout can work well.
    #     pass

    # def test_disconnected(self):
    #     pass

    def test_request(self):
        print 'test request'
        request = 'random message'
        response = client.request(request)
        self.assertEqual(request, response)

    @unittest.skipIf(in_develop_mode, 'skip')
    def test_double_connection(self):
        print 'Testing double connection'
        second_client = Client((HOST, PORT), None)
        request = 'vget /mode'
        response = second_client.request(request)
        self.assertEqual(response, request)



if __name__ == '__main__':
    # unittest.main()
    # suite = unittest.TestSuite()
    # suite.addTest(TestCommands())

    load = unittest.defaultTestLoader.loadTestsFromTestCase
    suite = load(TestCommands)
    suite = load(TestNetworkConnection)
    suite = load(TestBaseClient)
    # suite.run()
    unittest.TextTestRunner().run(suite)

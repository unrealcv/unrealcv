# TODO: Test robustness, test speed
import unittest, time, sys
from checker import *
from test_common import *
from test_server import EchoServer, MessageServer
import argparse
import threading

TESTPORT = PORT + 1
NO_PORT = PORT + 10 # TODO: Make it a random port
lock_ports = {}
for port in [PORT, TESTPORT]:
    lock_ports[port] = threading.RLock()

lock_unreal_server = threading.RLock()

in_develop_mode = False
# if '--develop' in sys.argv:
#     in_develop_mode = True
#     sys.argv.remove('--develop')
# else:
#     in_develop_mode = False

# client = Client((HOST, PORT), None) # Try different port number
# in_develop_mode = False

def run_tasks(testcase, client, tasks):
    for task in tasks:
        cmd = task[0]
        expect = task[1]

        print 'Cmd: %s' % cmd
        response = client.request(cmd)
        if response == None:
            testcase.assertTrue(False, 'Can not connect to UnrealCV server')
            return

        print 'Response: %s' % repr(response)
        # Need to lock until I got a reply
        # print reply

        if expect == None or isinstance(expect, str):
            testcase.assertEqual(response, expect, 'Expect %s, Response %s' % ( expect, response))
        else:
            testcase.assertTrue(expect(response))


class TestCommands(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.client = Client((HOST, PORT), None)

    # TODO: Test some error handling case
    def setUp(self): # this setUp will be run for each test function??
        pass
        # self.client = Client((HOST, PORT), None) # Try different port number

    @unittest.skipIf(in_develop_mode, 'skip')
    def test_objects(self):
        response = self.client.request('vget /objects')
        self.assertTrue(validate_format(response))

        objects = response.split(' ')
        self.assertTrue(len(objects) > 0)

        print 'Number of objects %d' % len(objects)
        tasks = []
        for objname in objects:
            tasks.append(['vget /object/%s/name' % objname, objname])
            tasks.append(['vget /object/%s/color' % objname, skip])
            # TODO: add a function to check regular expression

        run_tasks(self, self.client, tasks)

    @unittest.expectedFailure # TODO: Need to fix location
    # @unittest.skipIf(in_develop_mode, 'skip')
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

        run_tasks(self, self.client, tasks)

    @unittest.skipIf(in_develop_mode, 'skip')
    def test_viewmode(self):
        tasks = []
        modes = ['depth', 'lit', 'unlit', 'normal', 'object_mask']
        for mode in modes:
            tasks.append(['vset /mode/%s' % mode, 'ok']) # TODO: Change it to vset /mode modename
            tasks.append(['vget /mode', mode])

        run_tasks(self, self.client, tasks)

class TestBaseClient(unittest.TestCase):
    '''
    Test BaseClient
    '''
    @classmethod
    def setUpClass(cls):
        cls.port = TESTPORT
        lock_ports[cls.port].acquire()
        cls.server = MessageServer((HOST, cls.port))
        cls.server.start()
        # cls.base_client = BaseClient((HOST, PORT), None)

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        lock_ports[cls.port].release()

    def test_send(self):
        request = "Hello"
        num_response = [0] # This is a trick for workaround
        def handler(response):
            if response == request:
                num_response[0] += 1

        client1 = BaseClient((HOST, self.port), handler)
        client2 = BaseClient((HOST, self.port), handler)
        client3 = BaseClient((HOST, self.port), handler)
        client4 = BaseClient((HOST, self.port), handler)
        for client in [client1, client2, client3, client4]:
            sent= client.send(request)
            self.assertEqual(sent, True, 'Can not send message')

        time.sleep(0.1) # Tolerate some delay
        self.assertEqual(num_response[0], 4)

    def test_no_server(self):
        '''
        Test what will happen if no server is available
        Should give reasonable error
        '''
        client = BaseClient((HOST, NO_PORT), None)
        sent = client.send('hello')
        self.assertEqual(sent, False)


class TestClient(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = TESTPORT
        lock_ports[cls.port].acquire()
        cls.server = MessageServer((HOST, cls.port))
        cls.server.start()

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        lock_ports[cls.port].release()

    def test_request(self):
        '''
        Simple test to test basic function
        '''
        client = Client((HOST, self.port), None)
        tasks = [
            ['hi', 'hi'],
            ['hello', 'hello']
            ]

        run_tasks(self, client, tasks)

    # def test_multi_clients(self):
    #     clients = [Client((HOST, PORT)) for _ in range(5)] # create 5 clients

    def test_no_server(self):
        client = Client((HOST, NO_PORT), None)
        tasks = [
            ['hi', None],
            ['hello', None]
            ]

        run_tasks(self, client, tasks)

class TestRealisticRendering(unittest.TestCase):
    pass

class TestMessageServer(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.port = TESTPORT

    def test_server(self):
        with lock_ports[self.port]:
            print 'test server'
            server = MessageServer((HOST, self.port))
            server.start()
            server.shutdown()

    def test_release(self):
        with lock_ports[self.port]:
            print 'test release'
            server = MessageServer((HOST, self.port))
            server.start()
            server.shutdown()
            server = MessageServer((HOST, self.port))
            server.start()
            server.shutdown()

if __name__ == '__main__':
    load = unittest.defaultTestLoader.loadTestsFromTestCase

    parser = argparse.ArgumentParser()
    parser.add_argument('--travis', action='store_true') # Only run test availabe to travis CI

    args = parser.parse_args()

    # unittest.main()
    # suite = unittest.TestSuite()
    # suite.addTest(TestCommands())
    suites = []
    s = load(TestMessageServer); suites.append(s)
    s = load(TestBaseClient); suites.append(s)
    s = load(TestClient); suites.append(s)

    if not args.travis:
        s = load(TestCommands)
        suites.append(s)

    suite_obj = unittest.TestSuite(suites)
    # suite.run()
    unittest.TextTestRunner().run(suite_obj)

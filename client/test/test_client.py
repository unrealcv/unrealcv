import unittest, threading, random, logging
from common_conf import *
import ue4cv
from checker import *
from test_server import EchoServer, MessageServer, NullServer
L = logging.getLogger(__name__)

def random_payload():
    len = random.randrange(1024)
    random_str = ''.join([chr(random.randrange(100)) for v in range(len)])
    return random_str

def no_error(func):
    def call(*args, **kwargs):
        logging.getLogger('ue4cv').setLevel(logging.CRITICAL)
        result = func(*args, **kwargs)
        logging.getLogger('ue4cv').setLevel(logging.INFO)
        return result
    return call

class TestClientWithDummyServer(unittest.TestCase):
    '''
    Use python dummy server to test the robustness of ue4cv.Client
    '''
    @classmethod
    def setUpClass(cls):
        cls.echo_port = 9000 # Message sent to here will be echoed back
        cls.null_port = 9001 # Message sent to here will not get any reply
        cls.no_port = 9002 # No server is running on this port
        cls.host = 'localhost'
        cls.echo_server = MessageServer((cls.host, cls.echo_port))
        cls.echo_server.start()
        cls.null_server = NullServer((cls.host, cls.null_port))
        cls.null_server.start()

    @classmethod
    def tearDownClass(cls):
        cls.echo_server.shutdown()
        cls.null_server.shutdown()

    def test_request(self):
        '''
        Simple test to test basic function
        '''
        ue4cv.client.connect()
        self.assertEqual(ue4cv.client.isconnected(), True)
        tasks = [
            ['hi', 'hi'],
            ['hello', 'hello'],
            ['asdf'*70, 'asdf'*70], # TODO: Create some random strings
        ]
        for _ in range(5):
            request = random_payload()
            tasks.append([request, request])

        run_tasks(self, ue4cv.client, tasks)

    def test_multiple_connection(self):
        '''
        Only one client is allowed for the server
        Make a second connection to the server, when one connection exists
        '''
        ue4cv.client.disconnect()
        ue4cv.client.connect()
        self.assertTrue(ue4cv.client.isconnected())
        response = ue4cv.client.request('hi')
        self.assertEqual(response, 'hi')
        for i in range(10):
            client = ue4cv.Client((self.host, self.echo_port))
            client.connect()
            # print client.connect()
            self.assertEqual(client.isconnected(), False)

    def test_random_operation(self):
        for i in range(10):
            choice = random.randrange(2)
            if choice == 1:
                ue4cv.client.connect()
                self.assertEqual(ue4cv.client.isconnected(), True)
            elif choice == 0:
                ue4cv.client.disconnect()
                self.assertEqual(ue4cv.client.isconnected(), False)

        for i in range(10):
            ue4cv.client.connect()
            self.assertEqual(ue4cv.client.isconnected(), True)
        for i in range(10):
            ue4cv.client.disconnect()
            self.assertEqual(ue4cv.client.isconnected(), False)

    def test_client_release(self):
        '''
        If the previous client release the connection, further connection should be accepted. This will also test the server code
        '''
        ue4cv.client.disconnect()
        for _ in range(10):
            L.info('Try to connect')
            ue4cv.client.connect()
            self.assertEqual(ue4cv.client.isconnected(), True)

            request = 'ok'
            response = ue4cv.client.request(request)
            self.assertEqual(request, response)

            ue4cv.client.disconnect()
            self.assertEqual(ue4cv.client.isconnected(), False)

    @no_error
    def test_request_timeout(self):
        '''
        In which case the request will timeout?
        If the server did not respond with a correct reply.
        '''
        client = ue4cv.Client((self.host, self.null_port))
        client.connect()
        self.assertEqual(client.isconnected(), True)
        response = client.request('hi', timeout = 1)
        self.assertEqual(response, None)

    def test_stress(self):
        '''
        Create very large payload to see whether the connection is stable
        '''
        pass

    @no_error
    def test_no_server(self):
        # Suppress error message

        client = ue4cv.Client((self.host, self.no_port), None)
        client.connect()
        self.assertEqual(client.isconnected(), False)
        tasks = [
            ['hi', None],
            ['hello', None],
        ]

        run_tasks(self, client, tasks)


if __name__ == '__main__':
    logging.basicConfig()

    # suite = TestSuite()
    tests = [
        'test_client_release',
        'test_request',
        'test_multiple_connection',
        'test_random_operation',
        'test_stress',
        'test_no_server', # TODO, random order
        ]
    suite = unittest.TestSuite(map(TestClientWithDummyServer, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

    # unittest.main(verbosity = 3)

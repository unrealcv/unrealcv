import unittest, threading, random, logging
from common_conf import *
import unrealcv
from checker import *
from dev_server import EchoServer, MessageServer, NullServer
_L = logging.getLogger(__name__)

def random_payload():
    len = random.randrange(1024)
    random_str = ''.join([chr(random.randrange(100)) for v in range(len)])
    return random_str

def no_error(func):
    def call(*args, **kwargs):
        logging.getLogger('unrealcv').setLevel(logging.CRITICAL)
        result = func(*args, **kwargs)
        logging.getLogger('unrealcv').setLevel(logging.INFO)
        return result
    return call


class TestUE4CVClient(unittest.TestCase):
    '''
    Use python dummy server to test the robustness of unrealcv.Client, Pass the test of DevServer first
    '''
    num_random_trial = 10
    num_release_trial = 10

    @classmethod
    def setUpClass(cls):
        cls.echo_port = 9010
        # Message sent to here will be echoed back
        cls.null_port = 9011
        # Message sent to here will not get any reply
        cls.no_port = 9012
        # No server is running on this port
        cls.host = 'localhost'
        cls.echo_server = MessageServer((cls.host, cls.echo_port))
        cls.echo_server.start()
        cls.null_server = NullServer((cls.host, cls.null_port))
        cls.null_server.start()
        unrealcv.client = unrealcv.Client((cls.host, cls.echo_port))
        # time.sleep(1) # Wait for the server to get ready

    @classmethod
    def tearDownClass(cls):
        cls.echo_server.shutdown()
        cls.null_server.shutdown()

    def test_request(self):
        '''
        Simple test to test basic function
        '''
        unrealcv.client.connect()
        self.assertEqual(unrealcv.client.isconnected(), True)
        tasks = [
            ['hi', 'hi'],
            ['hello', 'hello'],
            ['asdf'*70, 'asdf'*70], # TODO: Create some random strings
        ]
        # for _ in range(5):
        #     request = random_payload()
        #     request.replace(':', '')
        #     tasks.append([request, request])

        run_tasks(self, unrealcv.client, tasks)

    def test_multiple_connection(self):
        '''
        Only one client is allowed for the server
        Make a second connection to the server, when one connection exists
        '''
        unrealcv.client.disconnect()
        unrealcv.client.connect(timeout=0.5)
        self.assertTrue(unrealcv.client.isconnected())
        response = unrealcv.client.request('hi')
        self.assertEqual(response, 'hi')
        for i in range(10):
            client = unrealcv.Client((self.host, self.echo_port))
            client.connect(0.1)
            # print client.connect()
            self.assertEqual(client.isconnected(), False)

    def test_random_operation(self):
        _L.info('Try random operation %d times', self.num_random_trial)
        for i in range(self.num_random_trial):
            msg = 'Trial %d' % i
            choice = random.randrange(2)
            if choice == 1:
                unrealcv.client.connect()
                self.assertEqual(unrealcv.client.isconnected(), True, msg)
            elif choice == 0:
                unrealcv.client.disconnect()
                self.assertEqual(unrealcv.client.isconnected(), False, msg)

        for i in range(10):
            unrealcv.client.connect()
            self.assertEqual(unrealcv.client.isconnected(), True)
        for i in range(10):
            unrealcv.client.disconnect()
            self.assertEqual(unrealcv.client.isconnected(), False)

    def test_client_release(self):
        '''
        If the previous client release the connection, further connection should be accepted. This will also test the server code
        '''
        unrealcv.client.disconnect()
        _L.info('Try client release %d times', self.num_random_trial)
        for _ in range(self.num_release_trial):
            msg = 'Trial %d' % _
            unrealcv.client.connect()
            self.assertEqual(unrealcv.client.isconnected(), True, msg)

            # Do something
            request = 'ok'
            response = unrealcv.client.request(request)
            self.assertEqual(request, response, msg)

            unrealcv.client.disconnect()
            self.assertEqual(unrealcv.client.isconnected(), False, msg)

    @no_error
    def test_request_timeout(self):
        '''
        In which case the request will timeout?
        If the server did not respond with a correct reply.
        '''
        client = unrealcv.Client((self.host, self.null_port))
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

        client = unrealcv.Client((self.host, self.no_port), None)
        client.connect()
        self.assertEqual(client.isconnected(), False)
        tasks = [
            ['hi', None],
            ['hello', None],
        ]

        run_tasks(self, client, tasks)

    def test_server_shutdown(self):
        '''
        Close on the server side and check whether client can detect it
        '''
        pass

    def test_change_port(self):
        pass

class TestReceiveHandler(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.echo_server = MessageServer(('localhost', 9000))
        cls.echo_server.start()

    def handle_message(self, msg):
        print 'Got server message %s' % repr(msg)
        res = unrealcv.client.request('ok', 1)
        self.assertEqual(res, 'ok')
        print 'Server response %s' % res

    def test_message_handler(self):
        unrealcv.client.connect()
        self.assertEqual(unrealcv.client.isconnected(), True)
        unrealcv.client.message_handler = self.handle_message

        res = unrealcv.client.request('ok')
        self.assertEqual(res, 'ok')

        self.echo_server.send('Hello from server')
        time.sleep(5)
        # TODO: write this in event fashion


if __name__ == '__main__':
    logging.basicConfig()
    logging.getLogger(__name__).setLevel(logging.INFO)
    # logging.getLogger('unrealcv').setLevel(logging.CRITICAL)
    logging.getLogger('unrealcv').setLevel(logging.INFO)

    TestUE4CVClient.num_random_trial = 100
    TestUE4CVClient.num_release_trial = 100
    # suite = TestSuite()
    # tests = [
    #     'test_random_operation',
    #     'test_request',
    #     'test_client_release',
    #     'test_multiple_connection',
    #     'test_stress',
    #     'test_no_server', # TODO, random order
    #     ]
    # suite = unittest.TestSuite(map(TestUE4CVClient, tests))
    # unittest.TextTestRunner(verbosity=2).run(suite)

    suite = unittest.TestLoader().loadTestsFromTestCase(TestReceiveHandler)
    unittest.TextTestRunner(verbosity=2).run(suite)
    # unittest.main(verbosity = 3)

import unittest, logging, argparse, random, socket, time
from common_conf import *
import ue4cv
_L = logging.getLogger(__name__)

class TestUE4CVServer(unittest.TestCase):
    '''
    Test the robustness of UE4CVServer
    Pass the test of UE4CVClient first
    '''
    port = 9000
    host = 'localhost'
    @classmethod
    def setUpClass(cls):
        print cls.port
        cls.test_cmd = 'vset /mode/depth'
        ue4cv.client = ue4cv.Client(('localhost', cls.port))
        ue4cv.client.connect()
        if not ue4cv.client.isconnected():
            raise Exception('Fail to connect to UE4CVServer, check whether server is running and whether a connection exists')

    def test_client_release(self):
        '''
        If the previous client release the connection, further connection should be accepted. This will also test the server code
        '''
        ue4cv.client.disconnect()
        for _ in range(10):
            _L.info('Try to connect')
            ue4cv.client.connect()
            self.assertEqual(ue4cv.client.isconnected(), True)

            response = ue4cv.client.request(self.test_cmd)
            self.assertEqual(response, 'ok')

            ue4cv.client.disconnect()
            self.assertEqual(ue4cv.client.isconnected(), False)

    def test_multiple_connection(self):
        ue4cv.client.disconnect()
        ue4cv.client.connect()
        self.assertTrue(ue4cv.client.isconnected())
        response = ue4cv.client.request(self.test_cmd)
        self.assertEqual(response, 'ok')
        for i in range(10):
            client = ue4cv.Client((self.host, self.port))
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

    def test_client_side_close(self):
        '''
        Test whether the server can correctly detect client disconnection
        '''
        for i in range(5): # TODO, change this number to 10
            print i
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((self.host, self.port))
            # s.sendall('Hello, world')
            # data = s.recv(1024)
            # How to know whether this s is closed by remote?
            ue4cv.SocketMessage.WrapAndSendPayload(s, 'hello')
            # No shutdown to simulate sudden close
            s.close() # It will take some time to notify the server
            # time.sleep(1) # Wait for the server to release the connection

if __name__ == '__main__':
    logging.basicConfig(level=logging.ERROR)

    parser = argparse.ArgumentParser()
    parser.add_argument('--port', help='Which port the game is running', default=9000, type=int)
    args = parser.parse_args()
    TestUE4CVServer.port = args.port

    tests = [
        'test_client_release',
        'test_multiple_connection',
        'test_random_operation',
        'test_client_side_close',
        ]
    suite = unittest.TestSuite(map(TestUE4CVServer, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

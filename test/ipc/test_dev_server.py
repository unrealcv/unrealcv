import threading, time, socket, unittest, logging, SocketServer
from common_conf import *
from dev_server import MessageServer, MessageTCPHandler
import unrealcv
_L = logging.getLogger(__name__)
_L.setLevel(logging.INFO)
_L.addHandler(logging.NullHandler())
# This is important

port_lock = threading.RLock()
class TestDevServer(unittest.TestCase):
    '''
    The test of development server
    '''
    @classmethod
    def setUpClass(cls):
        cls.port = 9001
        cls.host = 'localhost'

    def test_server(self):
        with port_lock:
            server = MessageServer((self.host, self.port))
            server.start()
            server.shutdown()

    def test_release(self):
        '''
        Test whether resources are correctly released
        '''
        with port_lock:
            for i in range(10):
                server = MessageServer((self.host, self.port))
                server.start()
                server.shutdown()

    def test_client_side_close(self):
        '''
        Test whether the server can correctly detect client disconnection
        '''
        with port_lock:
            server = MessageServer((self.host, self.port))
            server.start()

            for i in range(10):
                _L.info('Trial %d', i)
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect((self.host, self.port))
                # s.sendall('Hello, world')
                # data = s.recv(1024)
                # How to know whether this s is closed by remote?
                unrealcv.SocketMessage.WrapAndSendPayload(s, 'hello')
                s.close() # It will take some time to notify the server
                time.sleep(0.5) # How long will the server should detect the client side loss
                self.assertEqual(MessageTCPHandler.connected, False) # Check any client is connected

            server.shutdown()

if __name__ == '__main__':
    logging.basicConfig()
    logging.getLogger(__name__).setLevel(logging.ERROR)
    logging.getLogger('unrealcv').setLevel(logging.CRITICAL)
    # unittest.main(verbosity=2)

    tests = [
        'test_server',
        'test_release',
        'test_client_side_close',
        ]
    suite = unittest.TestSuite(map(TestDevServer, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

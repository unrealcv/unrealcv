import unittest
from common_conf import *
import ue4cv
_L = logging.getLogger(__name__)

class TestUE4CVServer(unittest.TestCase):
    '''
    Test the robustness of UE4CVServer
    Pass the test of UE4CVClient first
    '''
    @classmethod
    def setUpClass(cls):
        cls.port = 9000

    def test_multiple_connection(self):
        ue4cv.client.disconnect()
        ue4cv.client.connect()
        self.assertTrue(ue4cv.client.isconnected())
        response = ue4cv.client.request(self.test_cmd)
        self.assertEqual(response, 'ok')
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
            _L.info('Try to connect')
            ue4cv.client.connect()
            self.assertEqual(ue4cv.client.isconnected(), True)

            response = ue4cv.client.request(self.test_cmd)
            self.assertEqual(response, 'ok')

            ue4cv.client.disconnect()
            self.assertEqual(ue4cv.client.isconnected(), False)

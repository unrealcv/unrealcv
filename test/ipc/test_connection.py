import unittest, pytest
import random

import sys; sys.path.append('..')
from testcfg import client
# Very basic tests to make sure the connection between client and server is stable

class ConnectionTestCase(unittest.TestCase):
    def test_connect(self):
        '''
        Simply test the connection
        '''
        client.connect()
        self.assertTrue(client.isconnected())

    def test_multiple_attempt(self):
        '''
        Try to connect when a connection already exists
        '''
        for _ in range(10):
            client.connect()
            self.assertTrue(client.isconnected())

    @pytest.mark.skip(reason="no way of currently testing this")
    @unittest.skip("Not well implemented")
    def test_multiple_connection(self):
        '''
        Try to use multiple different clients to connect to the server.
        Only the first one should be accepted and the future client will be rejected. #TODO #feature
        '''
        client.disconnect()
        client.connect()
        self.assertTrue(client.isconnected())
        response = client.request(self.test_cmd)
        self.assertEqual(response, 'ok')
        for i in range(10):
            client = unrealcv.Client((self.host, self.port))
            # Create a different client instance
            client.connect()
            # print client.connect()
            self.assertEqual(client.isconnected(), False)

    def test_random_operation(self):
        '''
        Connect and disconnect randomly to simulate real world cases
        '''
        for i in range(10):
            choice = random.randrange(2)
            if choice == 1:
                client.connect()
                self.assertEqual(client.isconnected(), True)
            elif choice == 0:
                client.disconnect()
                self.assertEqual(client.isconnected(), False)

    def test_disconnect(self):
        '''
        Connect and disconnect multiple times, make sure the connection is successfully released
        '''
        for _ in range(5):
            client.connect()
            self.assertEqual(client.isconnected(), True)
            client.disconnect()
            self.assertEqual(client.isconnected(), False)

    @pytest.mark.skip(reason="no way of currently testing this")
    @unittest.skip("Not well implemented")
    def test_socket_close(self):
        '''
        Test whether the server can correctly detect socket errors
        '''
        for i in range(5): # TODO, change this number to 10
            print i
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect((self.host, self.port))
            # s.sendall('Hello, world')
            # data = s.recv(1024)
            # How to know whether this s is closed by remote?
            unrealcv.SocketMessage.WrapAndSendPayload(s, 'hello')
            # No shutdown to simulate sudden close
            s.close() # It will take some time to notify the server
            # time.sleep(1) # Wait for the server to release the connection


    def test_echo(self):
        '''
        Test `vget /unrealcv/echo` the most basic command of unrealcv
        '''
        import string
        # run an echo command, make sure I can get what I want
        client.connect()

        # Try to produce a very long random string sequence
        nTrial = 10
        for _ in range(nTrial):
            N = 1000 # Define a long string
            msg = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(N))
            res = client.request('vget /unrealcv/echo %s' % msg)
            self.assertEqual(res, msg)

if __name__ == '__main__':
    # unittest.main()

    suite = unittest.TestSuite()
    suite.addTest(ConnectionTestCase("test_disconnect"))
    suite.addTest(ConnectionTestCase("test_multiple_connect_attempt"))
    suite.addTest(ConnectionTestCase("test_echo"))
    unittest.TextTestRunner(verbosity=2).run(suite)

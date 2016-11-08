import unittest
import random

from testcfg import client
# Very basic tests to make sure the connection between client and server is stable

class ConnectionTestCase(unittest.TestCase):
    def test_connect(self):
        client.connect()
        self.assertTrue(client.isconnected())

    def test_multiple_connect_attempt(self):
        for _ in range(10):
            client.connect()
            self.assertTrue(client.isconnected())

    def test_multiple_connect_client(self):
        pass

    def test_disconnect(self):
        client.connect()
        self.assertEqual(client.isconnected(), True)
        client.disconnect()
        self.assertEqual(client.isconnected(), False)
        client.connect()
        self.assertEqual(client.isconnected(), True)

    def test_echo(self):
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

test_suite = unittest.TestLoader().loadTestsFromTestCase(ConnectionTestCase)
if __name__ == '__main__':
    # unittest.main()

    suite = unittest.TestSuite()
    suite.addTest(ConnectionTestCase("test_disconnect"))
    suite.addTest(ConnectionTestCase("test_multiple_connect_attempt"))
    suite.addTest(ConnectionTestCase("test_echo"))
    unittest.TextTestRunner(verbosity=2).run(suite)

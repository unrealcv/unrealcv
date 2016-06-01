import unittest
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

    def test_connect(self):
        client = Client((HOST, self.port))
        if client.connect():
            pass
        client2 = Client((HOST, self.port))
        self.assertEqual(client2.connect(), False)

    def test_shutdown(self):
        client.connect((host, port))

    def test_request_timeout(self):
        pass

    def test_double_connection(self):
        # Make a second connection to the server, when one connection exists
        pass

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

if __name__ == '__main__':
    unittest.main()

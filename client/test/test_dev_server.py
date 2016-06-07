import threading, time, socket, unittest, logging, SocketServer
from common_conf import *
import ue4cv
# import MySocketServer as SocketServer
SocketServer.ThreadingMixIn.daemon_threads = True
SocketServer.TCPServer.allow_reuse_address = True
_L = logging.getLogger(__name__)
_L.setLevel(logging.DEBUG)
_L.addHandler(logging.NullHandler())
# This is important

class ThreadedServer:
    def start(self):
        def _():
            cur_thread = threading.current_thread()
            _L.debug('Start in %s' % cur_thread.name)

            self.server.serve_forever()
            # Activate the server; this will keep running until you
            # interrupt the program with Ctrl-C

        import threading
        server_thread = threading.Thread(target = _)
        server_thread.setDaemon(1)
        server_thread.start() # TODO: stop this thread
        # time.sleep(0.1) # Wait for the server started

    def shutdown(self):
        cur_thread = threading.current_thread()
        _L.debug('Shutdown in %s' % cur_thread.name)
        self.server.shutdown()
        # try:
        #     self.server.socket.shutdown(socket.SHUT_RDWR)
        # except:
        #     pass
        self.server.server_close() # Close socket
        _L.debug('Shutdown completed')


class EchoTCPHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """
    def handle(self): # Return a socket when a new connection is started
        # self.request is the TCP socket connected to the client
        # Each handle is running in a seperate thread
        cur_thread = threading.current_thread()
        # print 'Got data in ', cur_thread.name
        while 1: # Need a way to stop the server
            data = self.request.recv(1024)  # Return whatever it gets
            if not data: # The connection is lost
                break
            # print "{} wrote:".format(self.client_address[0])
            # print data
            self.request.sendall(data)
        # print 'Close data thread ', cur_thread.name

connected = False
connected_lock = threading.RLock()
class MessageTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        thread_name = threading.current_thread().name
        _L.debug('Got a new connection from %s in %s' % (  self.request.getpeername(), thread_name))
        with connected_lock:
            global connected
            if connected:
                # SocketMessage.WrapAndSendPayload(self.request, "Only accept one connection")
                # Close socket, Disconnect request
                self.request.close()
                # self.request.close()
                _L.debug('Reject, only accept one connection')
                return
            else:
                ue4cv.SocketMessage.WrapAndSendPayload(self.request, 'connected to Python Message Server')
                _L.debug('Accept new connection')
                connected = True

        # t = threading.Thread(target = self.ticking_message)
        # t.setDaemon(True)
        # t.start()
        while 1: # Main loop to receive message
            _L.debug('Server looping in %s' % thread_name)
            message = ue4cv.SocketMessage.ReceivePayload(self.request)
            _L.debug('Server looping finished in %s' % thread_name)
            if not message:
                _L.debug('Server release connection in %s' % thread_name)
                connected = False
                break
            # SocketMessage.WrapAndSendPayload(self.request, 'reply')
            ue4cv.SocketMessage.WrapAndSendPayload(self.request, message)
            # SocketMessage.WrapAndSendPayload(self.request, 'got2')
        connected = False

    def ticking_message(self):
        while 1:
            # SocketMessage.WrapAndSendPayload(self.request, 'ticking from server')
            filename = '../data/1034.png'
            ue4cv.SocketMessage.WrapAndSendPayload(self.request, filename)
            time.sleep(1)

class NULLTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        ue4cv.SocketMessage.WrapAndSendPayload(self.request, 'connected to Python Null Server')
        while 1:
            message = ue4cv.SocketMessage.ReceivePayload(self.request)
            if not message:
                break

class EchoServer(ThreadedServer):
    def __init__(self, endpoint):
        self.endpoint = endpoint

        # Create the server, binding to localhost on port 9999
        # self.server = SocketServer.TCPServer(self.endpoint, EchoServer.MyTCPHandler)
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, EchoTCPHandler)

class MessageServer(ThreadedServer):
    def __init__(self, endpoint):
        self.endpoint = endpoint
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, MessageTCPHandler)

class NullServer(ThreadedServer):
    '''
    Message sent to here will get no response
    '''
    def __init__(self, endpoint):
        self.endpoint = endpoint
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, NULLTCPHandler)

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
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.connect((self.host, self.port))
                # s.sendall('Hello, world')
                # data = s.recv(1024)
                # How to know whether this s is closed by remote?
                ue4cv.SocketMessage.WrapAndSendPayload(s, 'hello')
                s.close() # It will take some time to notify the server
                time.sleep(1) # How long will the server should detect the client side loss
                self.assertEqual(connected, False)

            server.shutdown()

if __name__ == '__main__':
    logging.basicConfig(level=logging.ERROR)
    # unittest.main(verbosity=2)

    tests = [
        'test_server',
        'test_release',
        'test_client_side_close',
        ]
    suite = unittest.TestSuite(map(TestDevServer, tests))
    unittest.TextTestRunner(verbosity=2).run(suite)

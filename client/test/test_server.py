import threading, time
from test_common import *
# import MySocketServer as SocketServer
import SocketServer
SocketServer.ThreadingMixIn.daemon_threads = True
SocketServer.TCPServer.allow_reuse_address = True
import socket
# This is important

class EchoServer:
    '''
    Low level echo server, return whatever it got
    '''
    class MyTCPHandler(SocketServer.BaseRequestHandler):
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
            print 'Got data in ', cur_thread.name
            while 1: # Need a way to stop the server
                data = self.request.recv(1024)  # Return whatever it gets
                if not data: # The connection is lost
                    break
                # print "{} wrote:".format(self.client_address[0])
                # print data
                self.request.sendall(data)
            print 'Close data thread ', cur_thread.name

    def __init__(self, endpoint):
        self.endpoint = endpoint

        # Create the server, binding to localhost on port 9999
        # self.server = SocketServer.TCPServer(self.endpoint, EchoServer.MyTCPHandler)
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, EchoServer.MyTCPHandler)

    def start(self):
        def _():
            cur_thread = threading.current_thread()
            print 'Start in ', cur_thread.name

            self.server.serve_forever()
            # Activate the server; this will keep running until you
            # interrupt the program with Ctrl-C

        import threading
        server_thread = threading.Thread(target = _)
        server_thread.setDaemon(1)
        server_thread.start() # TODO: stop this thread
        time.sleep(0.1) # Wait for the server started

    def shutdown(self):
        cur_thread = threading.current_thread()
        print 'Shutdown in ', cur_thread.name
        print 'shutdown'
        self.server.shutdown()
        # try:
        #     self.server.socket.shutdown(socket.SHUT_RDWR)
        # except:
        #     pass
        self.server.server_close() # Close socket
        print 'shutdown completed'

class MessageServer:
    '''
    Receive a message and return exactly the same message back
    '''
    class MyTCPHandler(SocketServer.BaseRequestHandler):
        def handle(self):
            t = threading.Thread(target = self.ticking_message)
            t.setDaemon(True)
            t.start()
            while 1: # Main loop to receive message
                message = SocketMessage.ReceivePayload(self.request)
                if not message:
                    break
                # SocketMessage.WrapAndSendPayload(self.request, 'reply')
                SocketMessage.WrapAndSendPayload(self.request, message)
                # SocketMessage.WrapAndSendPayload(self.request, 'got2')

        def ticking_message(self):
            while 1:
                # SocketMessage.WrapAndSendPayload(self.request, 'ticking from server')
                filename = '../data/1034.png'
                SocketMessage.WrapAndSendPayload(self.request, filename)
                time.sleep(1)

    def __init__(self, endpoint):
        self.endpoint = endpoint

    def start(self):
        # Create the server, binding to localhost on port 9999
        server = SocketServer.TCPServer(self.endpoint, MessageServer.MyTCPHandler)

        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()

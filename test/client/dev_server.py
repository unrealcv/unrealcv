'''
A python server to mimic the behavior of unrealcv server
Useful for development
'''
import threading, logging, sys, socket
if (sys.version_info > (3, 0)):
    import socketserver as SocketServer
else:
    import SocketServer
# import MySocketServer as SocketServer
SocketServer.ThreadingMixIn.daemon_threads = True
# SocketServer.TCPServer.allow_reuse_address = True
SocketServer.TCPServer.allow_reuse_address = False

import time
import unrealcv

_L = logging.getLogger(__name__)
_L.setLevel(logging.INFO)
_L.addHandler(logging.NullHandler())

class ThreadedServer(object):
    def start(self):
        def _():
            cur_thread = threading.current_thread()
            _L.debug('Start in %s' % cur_thread.name)

            self.server.serve_forever()
            # Activate the server; this will keep running until you
            # interrupt the program with Ctrl-C

        self.server_thread = threading.Thread(target = _)
        self.server_thread.setDaemon(1)
        self.server_thread.start() # TODO: stop this thread
        time.sleep(0.1) # Wait for the server started

    def shutdown(self):
        cur_thread = threading.current_thread()
        _L.debug('Shutdown in %s' % cur_thread.name)
        # According to https://github.com/python/cpython/blob/master/Lib/socketserver.py
        self.server.shutdown() # Make a shutdown request
        self.server.server_close() # Do nothing, need to be override
        # This will try to stop the extra thread.

        try:
            self.server.socket.shutdown(socket.SHUT_RDWR)
        except:
            pass
        self.server.socket.close() # Explicitly close the socket
        time.sleep(0.5)
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

class MessageTCPHandler(SocketServer.BaseRequestHandler):
    # An unusual implementation
    connected = False
    client_socket = None

    def handle(self):
        client_socket = self.request

        thread_name = threading.current_thread().name
        _L.debug('Got a new connection from %s in %s' % (  client_socket.getpeername(), thread_name))
        # This client_socket must be able to handle the remote disconnection!
        client_socket.setblocking(1)

        if MessageTCPHandler.connected:
            client_socket.close()
            print('Reject, only accept one connection')
            return
        else:
            unrealcv.SocketMessage.WrapAndSendPayload(client_socket, 'connected to Python Message Server')
            print('Accept new connection')
            MessageTCPHandler.connected = True
            MessageTCPHandler.socket = client_socket

        while 1: # Main loop to receive message
            _L.debug('Server looping in %s' % thread_name)
            message = unrealcv.SocketMessage.ReceivePayload(client_socket)
            _L.debug('Server looping finished in %s' % thread_name)
            if not message:
                _L.debug('Server release connection in %s' % thread_name)
                MessageTCPHandler.connected = False
                MessageTCPHandler.socket = None
                break
            # SocketMessage.WrapAndSendPayload(self.request, 'reply')
            unrealcv.SocketMessage.WrapAndSendPayload(client_socket, message)
            # SocketMessage.WrapAndSendPayload(self.request, 'got2')

    @classmethod
    def send(cls, message):
        if cls.connected:
            unrealcv.SocketMessage.WrapAndSendPayload(cls.client_socket, message)

class NULLTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        unrealcv.SocketMessage.WrapAndSendPayload(self.request, 'connected to Python Null Server')
        while 1:
            message = unrealcv.SocketMessage.ReceivePayload(self.request)
            if not message:
                break

class EchoServer(ThreadedServer):
    '''
    Everything will be sent back
    '''
    def __init__(self, endpoint):
        self.endpoint = endpoint

        # Create the server, binding to localhost on port 9999
        # self.server = SocketServer.TCPServer(self.endpoint, EchoServer.MyTCPHandler)
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, EchoTCPHandler)

class MessageServer(ThreadedServer):
    '''
    Simulate sending a message with the correct format
    '''
    def __init__(self, endpoint):
        self.endpoint = endpoint
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, MessageTCPHandler)

    def send(self, message):
        MessageTCPHandler.send(message)

    def shutdown(self):
        super(MessageServer, self).shutdown()
        MessageTCPHandler.connected = False # FIXME: This is a very tricky implementation


class NullServer(ThreadedServer):
    '''
    Message sent to here will get no response
    '''
    def __init__(self, endpoint):
        self.endpoint = endpoint
        self.server = SocketServer.ThreadingTCPServer(self.endpoint, NULLTCPHandler)


if __name__ == '__main__':
    import logging
    L = logging.getLogger('unrealcv')
    L.setLevel(logging.DEBUG)

    logging.basicConfig()
    server = MessageServer(('localhost', 9000))
    server.start()
    while(1):
        pass

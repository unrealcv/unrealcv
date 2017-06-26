'''
A python server to mimic the behavior of unrealcv server
Useful for development
Make sure socket is properly closed and released, this is platform dependent and tricky

- NullServer : Do nothing when got a message
- EchoServer : Sendback everything it received
- MessageServer : Provide a send function and also check the message format from the client.

https://hg.python.org/cpython/file/2.7/Lib/SocketServer.py
'''
import threading, logging, sys, socket, time
import unrealcv
if (sys.version_info > (3, 0)):
    import socketserver as SocketServer
else:
    import SocketServer
# import MySocketServer as SocketServer
SocketServer.ThreadingMixIn.daemon_threads = True
# SocketServer.TCPServer.allow_reuse_address = True
SocketServer.TCPServer.allow_reuse_address = False

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
logger.addHandler(logging.StreamHandler(sys.stdout))

class ThreadedServer(SocketServer.TCPServer):
    # ThreadingTCPSever will use one thread for each client, this is not what I need, I need the server itself start in another thread
    '''
    Why a threaded server is needed. This server needs to be started in another thread and be used to notify the client.
    '''
    def __init__(self, server_address, RequestHandlerClass):
        '''
        Bind the server socket
        The socket will be released when this variable is released
        '''
        # similar to https://hg.python.org/cpython/file/2.7/Lib/SocketServer.py#L411
        SocketServer.TCPServer.__init__(self, server_address, RequestHandlerClass)

        def _():
            cur_thread = threading.current_thread()
            logger.info('Started in thread %s' % cur_thread.name)

            # self.serve_forever() # Copy from the definition from serve_forever
            while True:
                if self.running:
                    try:
                        request, client_address = self.get_request()
                    except socket.error:
                        return
                    self.RequestHandlerClass(request, client_address, self) # It might hang over here
                else:
                    pass
                    # logger.debug('Do nothing because it is stopped')
            logger.info("The server thread is stopped")

        self.running = False
        self.server_thread = threading.Thread(target = _)
        self.server_thread.setDaemon(1)
        self.server_thread.start() # Start can be only called once, even the _target has finished

    def start(self):
        '''
        Receive new connections
        '''
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        self.running = True
        time.sleep(0.1)
        # time.sleep(0.1) # Wait for the server started

    def stop(self):
        '''
        Stop new connections
        This will not release existing connections
        Stop should not release the server socket, but the socket binding is done in __init__, not in start
        '''
        self.running = False
        time.sleep(0.1)
        # Make sure we can successfully stop the serve_forever loop.
        # self.shutdown() # Make a shutdown request
        # self.server_close() # Do nothing, need to be override
        # This will try to stop the extra thread.

        # try:
        #     self.socket.shutdown(socket.SHUT_RDWR)
        # except:
        #     pass
        # self.socket.close() # Explicitly close the socket
        # time.sleep(0.1)
        # logger.debug('Shutdown completed')

class NULLTCPHandler(SocketServer.BaseRequestHandler):
    ''' Request handler that do nothing '''
    def handle(self):
        unrealcv.SocketMessage.WrapAndSendPayload(self.request, 'connected to Python Null Server')
        while 1:
            message = unrealcv.SocketMessage.ReceivePayload(self.request)
            if not message:
                break

class NullServer(ThreadedServer):
    ''' Message sent to here will get no response '''
    def __init__(self, endpoint):
        ThreadedServer.__init__(self, endpoint, NULLTCPHandler)

class EchoTCPHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """
    def handle(self):
        # self.request is the TCP socket connected to the client
        while 1: # Need a way to stop the server
            data = self.request.recv(1024)  # Return whatever it gets
            if not data: # The connection is lost
                break
            self.request.sendall(data)

class EchoServer(ThreadedServer):
    ''' Everything will be sent back '''
    def __init__(self, endpoint):
        ThreadedServer.__init__(self, endpoint, EchoTCPHandler)

class MessageTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        # Can we update the server information from the TCP handler?
        # Yes, use self.server
        assert self.request != None
        logger.debug('Got a connection from %s' % str(self.request.getpeername()))
        if self.server.client_socket:
            logger.debug('Reject, only accept one connection')
            self.request.close()
            return
        else:
            logger.debug('Accept, new connection')
            self.server.client_socket = self.request
            # self.request.setblocking(1)
            unrealcv.SocketMessage.WrapAndSendPayload(self.request, 'connected to Python Message Server')

        while True: # Main loop to receive message
            try:
                message = unrealcv.SocketMessage.ReceivePayload(self.request)
            except Exception as e:
                if e.errno == 10054:
                    logger.debug('Remote connection is forcibly closed')
                    message = None
            if message:
                unrealcv.SocketMessage.WrapAndSendPayload(self.request, message)
            else:
                logger.debug('Client release connection')
                self.server.client_socket = None
                break

class MessageServer(ThreadedServer):
    ''' Simulate sending a message with the correct format '''
    def __init__(self, endpoint):
        self.client_socket = None
        ThreadedServer.__init__(self, endpoint, MessageTCPHandler)

    def send(self, message):
        if self.client_socket:
            unrealcv.SocketMessage.WrapAndSendPayload(self.client_socket, message)

    def shutdown(self):
        # # TODO: Stop the message handler and do not accept any new connection during the shutdown connection
        # self.socket.close() # Make sure no new connection is coming during shutdown
        # print('Close client socket')
        # if self.client_socket:
        #     self.client_socket.shutdown(socket.SHUT_RDWR)
        #     self.client_socket.close()
        # time.sleep(0.5) # Make sure the handler stops
        #
        # print('Close server socket')
        # self.socket.close()
        # Shutdown will wait until the handle function returns
        ThreadedServer.shutdown(self)


if __name__ == '__main__':
    print('Run a null dev server for 2s for testing')
    server = NullServer(('localhost', 9000))
    server.start()
    server.stop()

    print('Run a Echo dev server for 2s for testing')
    server = EchoServer(('localhost', 9000))
    server.start()
    server.stop()

    print('Run a message server')
    server = MessageServer(('localhost', 9000))
    server.start()
    server.stop()
    # import logging
    # L = logging.getLogger('unrealcv')
    # L.setLevel(logging.DEBUG)
    #
    # logging.basicConfig()
    # server = MessageServer(('localhost', 9000))
    # server.start()
    # while(1):
    #     pass

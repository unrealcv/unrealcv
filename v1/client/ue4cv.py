import socket as S
import threading as TD
import logging as L

class Client:
    '''
    Start a tcp client to communicate with the Unreal demo server
    '''
    # Handle exception from the server, if there is an error message from the server, tell end-user
    # Parse warning and error message from the server
    def __init__(self, server_ip, server_port):
        self.server_ip = server_ip
        self.server_port = server_port
        self.num_attempt = 0
        self.interval = 1
        self.client_socket = None
        self.message_id = 0

    def set_connectd_handler(self, connected_handler):
        self.connected_handler = connected_handler

    def set_received_handler(self, received_handler):
        self.received_handler = received_handler

    def on_received(self, data):
        if self.__dict__.get('received_handler'):
            self.received_handler(data)
        else:
            L.warning('Received handler not set')

    def on_connected(self):
        # TODO: Make sure user input is valid
        if self.__dict__.get('connected_handler'):
            self.connected_handler()
        else:
            L.warning('Connected handler not set')

    def start_connection(self):
        '''
        Connect with Unreal Server, make sure the message from the server is as expected.
        Handle exception
        This is an async function
        '''
        # self._send_heartbeat()
        self.endpoint = (self.server_ip, self.server_port)
        self._check_server() # This will block execution

    def recv_basic(self):
        total_data = []
        while True:
            data = self.client_socket.recv(8192)
            print data
            if not data: break
            total_data.append(data)

        L.debug('Done receiving')
        return ''.join(total_data)

    def send(self, message):
        '''
        Send message to the unreal server
        '''
        print "Start a new socket to send message"
        client_socket = S.socket(S.AF_INET, S.SOCK_STREAM)
        client_socket.connect(self.endpoint)
        if client_socket:
            raw_message = '%d:%s' % (self.message_id, message)
            client_socket.sendall(raw_message)
            # data = self.client_socket.recv(5 * 1024 * 1024) # How to read all message?
            print 'Sent'
            data = self.recv_basic()
            reply = str(data)
            success_reply = '%d:ok' % self.message_id
            error_reply = '%d:error' % self.message_id

            if (reply.startswith('%d:' % self.message_id)):
                if (reply.startswith(success_reply)):
                    return True
                elif (reply.startswith(error_reply)):
                    logging.error(error_reply)
                    self.on_received(reply)
                else:
                    return reply
            else:
                self.on_received(reply)

            self.message_id += 1

    def _connect_server(self):
        '''
        Connect to server
        '''
        # print 'Try to connect,', endpoint
        # # TODO: Only maintain one socket and  one connection
        endpoint = (self.server_ip, self.server_port + 1)
        self.client_socket = S.socket(S.AF_INET, S.SOCK_STREAM) # This is used to receiving data
        self.client_socket.connect(endpoint)
        print 'Connected'
        print 'Waiting data from server'

        self.on_connected()

        while 1:
            data = self.client_socket.recv(1024)
            if not data:
                break # Disconnect

            self.on_received(data)

        self.client_socket.close() # Disconnect from the server
        print 'The server socket is closed' # TODO: replace print with python logging system

    def _check_server(self):
        L.info('Checking server: %s:%d, count: %d' % (self.server_ip, self.server_port, self.num_attempt))

        try:
            self._connect_server()
        except S.error as e: # only handle Socket error
            print 'Server is offline'
            print e

        self.num_attempt += 1
        t1 = TD.Timer(self.interval, self._check_server) # Check again
        t1.start()




if __name__ == '__main__': # Unit test
    pass

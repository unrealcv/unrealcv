# Start a listening server to simulate the behavior of the Unreal program.
'''
A debug server
'''
import socket as S
import thread as T
import threading as TS
import argparse

buffer_size = 1024
# host = '127.0.0.1'
client_socket = None

class DummyServer:
    def __init__(self, server_ip, server_port):
        self.server_ip = server_ip
        self.server_port = server_port
        self.client_socket = None

        endpoint = (self.server_ip, self.server_port)
        server_socket = S.socket(S.AF_INET, S.SOCK_STREAM)
        server_socket.bind(endpoint)
        server_socket.listen(1) # Only allow one connection
        self.server_socket = server_socket

        T.start_new_thread(self.send_handler, (self,))

    def recv_handler(self, client_socket, client_address):
        print 'Client is connected'
        import re
        message_re = re.compile('\d{1:8}:.*')
        while 1:
            data = client_socket.recv(buffer_size)
            message = repr(data)
            match = message_re.match(message) # Use regular expression to match
            if match:
                message_id = match.group(1)
                reply = '%s:ok' % message_id
                client_socket.sendall(reply)

            print 'data:' + repr(data)

            if not data:
                break

        client_socket.close()
        self.listen_server()

    def send_handler(self, _):
        while 1:
            message = raw_input('Type a message to send: ')
            print 'The message is: ', message
            if self.client_socket:
                self.client_socket.sendall(message) # Check whether this client is still connected?
            else:
                print 'Client is not connected'

    def listen_server(self):
        print 'Listening %s:%s' % (self.server_ip, self.server_port)
        print 'Waiting for connection'
        # global client_socket
        client_socket, client_address = self.server_socket.accept() # Block
        self.client_socket = client_socket
        # Keep this socket alive until I explictly disconnect it
        print '... Connected from:', client_address
        T.start_new_thread(self.recv_handler, (client_socket, client_address)) # How to handle the self? TODO:

        # timer = TS.Timer(1, listen_server)
        # timer.start()

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', default=8000, type=int)
    args = parser.parse_args()

    host = '0.0.0.0'
    server = DummyServer(host, args.port)
    server.listen_server()
    while (1):
        pass

    # Start a thread for interaction

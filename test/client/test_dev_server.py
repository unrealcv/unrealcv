'''
Test the correctness of the dev server, which simulates the functions of unrealcv server in python
'''
import threading, time, socket, unittest, logging, sys
if (sys.version_info > (3, 0)):
    import socketserver as SocketServer
else:
    import SocketServer
from dev_server import MessageServer, MessageTCPHandler
import unrealcv

host = 'localhost'
port = 9001

def test_server():
    server = MessageServer((host, port))
    server.start()
    server.shutdown()

def test_release():
    '''
    Test whether resources are correctly released
    '''
    for i in range(10):
        server = MessageServer((host, port))
        server.start() # Make sure the port has been released, otherwith this will fail
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        server.shutdown()

def test_client_side_close():
    '''
    Test whether the server can correctly detect client disconnection
    '''
    server = MessageServer((host, port))
    server.start()

    for i in range(10):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))

        unrealcv.SocketMessage.WrapAndSendPayload(s, 'hello')
        s.close() # It will take some time to notify the server
        time.sleep(0.5) # How long will the server should detect the client side loss

    server.shutdown()

if __name__ == '__main__':
    test_server()
    test_release()
    test_client_side_close()

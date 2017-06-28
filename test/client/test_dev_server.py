'''
Test the correctness of the dev server, which simulates the functions of unrealcv server in python
'''
import threading, time, socket, unittest, logging, sys
if (sys.version_info > (3, 0)):
    import socketserver as SocketServer
else:
    import SocketServer
from dev_server import MessageServer, MessageTCPHandler
import unrealcv, pytest

logger = logging.getLogger(__name__)
logger.setLevel(logging.INFO)
logger.addHandler(logging.StreamHandler(sys.stdout))

host = 'localhost'
port = 9001

def is_port_open(port):
    ''' Check whether a port of localhost is in use '''
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    try:
        s.bind(('localhost', port))
        s.close()
        return True
    except socket.error as e:
        errnos = [98, 48, 10048]
        if e.errno in errnos:
            logger.debug('Port %d in use' % port)
        else:
            logger.debug('Something unexpected happened in check_port')
            logger.debug(e)
        s.close()
        return False

# @pytest.mark.skip(reason = 'No useful')
def test_server_close():
    ''' Test whether resources are correctly released '''
    assert is_port_open(port) == True
    server = MessageServer((host, port)) # This wil bind to this endpoint
    assert is_port_open(port) == False

    for i in range(10):
        logger.debug('Run trial %d' % i)
        server.start() # Make sure the port has been released, otherwith this will fail

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))
        s.close() # FIXME: It is important to close all clients first, otherwise I can not shutdown the server

        logger.debug('Stop server, trial %d' % i)
        server.stop() # TODO: If server is stopped, we can not connect to it, use unrealcv client to test it!
        # time.sleep(0.1) # Wait until the connection is started
    # server.socket.close() # Explicitly release the server socket
    server.shutdown()

@pytest.mark.skip('Moved to test_client.py')
def test_client_close():
    '''
    Check whether the server can correctly detect
    - client connection and
    - client disconnection
    '''
    server = MessageServer((host, port))
    server.start()

    # Need to wait a while for the server to handle to connection and disconnection, it depends on your speed

    # for i in range(10):
    for i in range(3): # NOTE: Too many iterations like 10 is very likely to fail and not neccessary
        logger.debug('Trial %d' % i)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        logger.debug('Connect to remote socket')
        s.connect((host, port))
        # return until the remote accept the socket or not
        # But the socket has not been correctly handled in the Handler
        # which means the server status has not been updated yet!
        logger.debug('Connected to remote server socket')

        # Wait until the socket is correctly handled
        assert server.get_client_socket() != None

        unrealcv.SocketMessage.WrapAndSendPayload(s, 'hello')
        logger.debug('Close client socket')
        s.close() # It will take some time to notify the server.
        # There is no way to tell whether the server already detected the client socket behavior

        time.sleep(0.5)
        # NOTE: The server needs to be able to detect the client disconnection is a very short time, but there is no guarantee for it.
        # So it might be possible to get an invalid client socket.
        assert server.get_client_socket() == None

    # server.socket.close()
    # server.stop()
    server.shutdown()

if __name__ == '__main__':
    test_server_close()
    test_client_close()

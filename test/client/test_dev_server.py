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

if __name__ == '__main__':
    test_server_close()

from dev_server import MessageServer
import logging
L = logging.getLogger('unrealcv')
L.setLevel(logging.DEBUG)

logging.basicConfig()
server = MessageServer(('localhost', 9000))
server.start()
while(1):
    pass

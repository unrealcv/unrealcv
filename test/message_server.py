''' Return a message server to handle framed message '''
from test_common import *
import SocketServer

class MyTCPHandler(SocketServer.BaseRequestHandler):
    def handle(self):
        while 1: # Main loop to receive message
            if not SocketMessage.ReceivePayload(self.request):
                break
            SocketMessage.WrapAndSendPayload(self.request, 'got')
            # SocketMessage.WrapAndSendPayload(self.request, 'got2')


if __name__ == '__main__':
    print 'Start server %s:%d' % (HOST, PORT)

    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
    server.serve_forever()

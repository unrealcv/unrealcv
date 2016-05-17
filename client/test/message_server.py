''' Return a message server to handle framed message '''
from test_common import *
import SocketServer
import time, threading, os

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
            fullfilename = os.path.abspath(filename)
            SocketMessage.WrapAndSendPayload(self.request, filename)
            time.sleep(1)

if __name__ == '__main__':
    print 'Start server %s:%d' % (HOST, PORT)

    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
    server.serve_forever()

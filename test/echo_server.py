''' Run an echo server which reply everything it got '''
from test_common import *
import SocketServer

class MyTCPHandler(SocketServer.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self): # Return a socket when a new connection is started
        # self.request is the TCP socket connected to the client
        while 1:
            data = self.request.recv(1024)  # Return whatever it gets
            if not data: # The connection is lost
                break
            print "{} wrote:".format(self.client_address[0])
            print data
            self.request.sendall(data)
        # just send back the same data, but upper-cased

if __name__ == "__main__":
    print 'Start server %s:%d' % (HOST, PORT)

    # Create the server, binding to localhost on port 9999
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()

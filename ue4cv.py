

class SocketMessage:
    def __init__(self, content):
        pass


    def __str__(self):
        # Convert
        len_content = len(self.content) # Convert this to four byte uint32
        return len(self.content) + self.content

class Client:
    def __init__(self, host, port):
        self.socket = None # Todo create a socket
        pass

    def is_connected(self):
        return False # Add connection check

    def on_received(self, socket_message):
        # Receive one socket message a time, should I use the first few bits?

        pass

    # Start a thread to wait for data

    def _sendraw(self, raw_message):
        pass

    def read(self):
        pass

    def request(self, message):
        '''
        Make a request and return response
        '''
        self.send(message)
        reply_message = self.read() # Maybe this is not the reply?
        return reply_message.content

    def send(self, message):
        if self.is_connected():
            socket_message = SocketMessage(message)
            self._sendraw(str(socket_message))

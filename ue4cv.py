import ctypes, struct

fmt = 'I'

class SocketMessage:
    # This magic number is from Unreal implementation
    # See https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Source/Runtime/Sockets/Public/NetworkMessage.h
    magic = ctypes.c_uint32(0x9E2B83C1).value
    def __init__(self, payload):
        self.magic = SocketMessage.magic
        self.payload_size = ctypes.c_uint32(len(payload)).value

    @classmethod
    def ReceivePayload(cls, socket):
        '''
        Return only payload, not the raw message, None if failed
        '''
        rbufsize = -1 # From SocketServer.py
        rfile = socket.makefile('rb', rbufsize)
        raw_magic = rfile.read(4)
        # print 'Receive raw magic: %d, %s' % (len(raw_magic), raw_magic)
        magic = struct.unpack('I', raw_magic)[0]
        # print 'Receive magic:', magic

        if magic != cls.magic:
            L.error('Error: receive a malformat message, the message should start from a four bytes uint32 magic number')
            return None
            # The next time it will read four bytes again

        raw_payload_size = rfile.read(4)
        # print 'Receive raw payload size: %d, %s' % (len(raw_payload_size), raw_payload_size)
        payload_size = struct.unpack('I', raw_payload_size)[0]
        # print 'Receive', payload_size

        # if the message is incomplete, should wait until all the data received
        payload = ""
        size = payload_size
        while size > 0:
            data = rfile.read(payload_size)
            payload += data
            size -= len(data)

        rfile.close()

        return payload

    @classmethod
    def WrapAndSendPayload(cls, socket, payload):
        '''
        Send payload, true if success, false if failed
        '''
        # From SocketServer.py
        # wbufsize = 0, flush immediately
        wbufsize = -1
        # Convert
        socket_message = SocketMessage(payload)
        wfile = socket.makefile('wb', wbufsize)
        # Write the message
        wfile.write(struct.pack(fmt, socket_message.magic))
        # Need to send the packed version
        # print 'Sent ', socket_message.magic

        wfile.write(struct.pack(fmt, socket_message.payload_size))
        # print 'Sent ', socket_message.payload_size

        wfile.write(payload)
        # print 'Sent ', payload
        wfile.flush()
        wfile.close() # Close file object, not close the socket
        return True

class Client:
    '''
    Add message framing and CRC check on top of TCP
    '''
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

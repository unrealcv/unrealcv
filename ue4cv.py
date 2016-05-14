import ctypes, struct, threading, socket

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
        if not raw_magic:
            return None
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

class MessageClient:
    '''
    Add message framing on top of TCP
    '''
    def __init__(self, endpoint, message_handler):
        self.endpoint = endpoint
        self.message_handler = message_handler
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.is_connected = False
        self.connect()

    def connect(self):
        if not self.is_connected:
            try:
                self.socket.connect(self.endpoint)
                self.is_connected = True
                # Start a thread to get data from the socket
                receiving_thread = threading.Thread(target = self.receiving)
                receiving_thread.setDaemon(1)
                receiving_thread.start()
            except:
                print 'Can not connect to %s' % str(self.endpoint)
                self.is_connected = False

        return self.is_connected

    def receiving(self):
        while (1):
            # Only this thread is allowed to read from socket, otherwise need lock to avoid competing
            message = SocketMessage.ReceivePayload(self.socket)
            if not message:
                self.is_connected = False
                break
            self.message_handler(message)

    def send(self, message):
        if self.connect():
            SocketMessage.WrapAndSendPayload(self.socket, message)
            reply = 'reply'
            return reply
        else:
            return None

class Client:
    def raw_message_handler(self, raw_message):
        print raw_message
        if raw_message.find(':') != -1:
            [message_id, message] = raw_message.split(':')
            if int(message_id) == self.message_id:
                self.reply_event.set()
                self.reply = message
            else:
                assert(False)
        else:
            self.message_handler(raw_message)

    def __init__(self, endpoint, message_handler):
        self.message_client = MessageClient(endpoint, self.raw_message_handler)
        self.message_handler = message_handler
        self.message_id = 0
        self.reply_event = threading.Event()
        self.reply = ''

    def send(self, message):
        raw_message = '%8d:%s' % (self.message_id, message)
        self.message_client.send(raw_message)
        # Timeout is required
        # see: https://bugs.python.org/issue8844
        self.reply_event.wait(60 * 60)
        return self.reply

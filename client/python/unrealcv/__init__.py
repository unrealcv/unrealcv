'''
UnrealCV
========
Provides functions to interact with games built using Unreal Engine.

>>> import unrealcv
>>> (HOST, PORT) = ('localhost', 9000)
>>> client = unrealcv.Client((HOST, PORT))
'''
import sys, ctypes, struct, threading, socket, re, time, logging
try:
    from Queue import Queue
except:
    from queue import Queue # for Python 3
_L = logging.getLogger(__name__)
# _L.addHandler(logging.NullHandler()) # Let client to decide how to do logging
_L.handlers = []
h = logging.StreamHandler()
h.setFormatter(logging.Formatter('%(levelname)s:%(module)s:%(lineno)d:%(message)s'))
_L.addHandler(h)
_L.propagate = False
_L.setLevel(logging.INFO)

__version__ = '0.4.0'

class SocketMessage(object):
    '''
    Define the format of a message. This class is defined similar to the class FNFSMessageHeader in UnrealEngine4, but without CRC check.
    The magic number is from Unreal implementation
    See https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Source/Runtime/Sockets/Public/NetworkMessage.h
    '''
    magic = ctypes.c_uint32(0x9E2B83C1).value
    fmt = 'I'
    def __init__(self, payload):
        self.payload_size = ctypes.c_uint32(len(payload)).value

    @classmethod
    def ReceivePayload(cls, sock):
        '''
        Return only payload, not the raw message, None if failed.
        sock: a blocking socket for read data.
        '''
        # print(sock.gettimeout())
        # rbufsize = -1 # From SocketServer.py
        # rbufsize = 0
        # rfile = sock.makefile('rb', rbufsize)
        try:
            # raw_magic = rfile.read(4) # socket is disconnected or invalid
            raw_magic = b''
            while len(raw_magic) < 4:
                raw_magic += sock.recv(4)
        except Exception as e:
            _L.debug('Fail to read raw_magic, exception: "%s"', e)
            raw_magic = None

        _L.debug('Read raw_magic %s', repr(raw_magic))
        if not raw_magic: # nothing to read
            # _L.debug('socket disconnect')
            return None
        magic = struct.unpack(cls.fmt, raw_magic)[0] # 'I' means unsigned int

        if magic != cls.magic:
            _L.error('Error: receive a malformat message, the message should start from a four bytes uint32 magic number')
            return None
            # The next time it will read four bytes again

        _L.debug('read payload')
        # raw_payload_size = rfile.read(4)
        raw_payload_size = sock.recv(4)
        # print 'Receive raw payload size: %d, %s' % (len(raw_payload_size), raw_payload_size)
        payload_size = struct.unpack('I', raw_payload_size)[0]
        _L.debug('Receive payload size %d', payload_size)

        # if the message is incomplete, should wait until all the data received
        payload = b""
        remain_size = payload_size
        while remain_size > 0:
            # data = rfile.read(remain_size)
            data = sock.recv(remain_size)
            if not data:
                return None

            payload += data
            bytes_read = len(data) # len(data) is its string length, but we want length of bytes
            # print 'bytes_read %d, remain_size %d, read_str %s' % (bytes_read, remain_size, data)
            assert(bytes_read <= remain_size)
            remain_size -= bytes_read

        return payload

    @classmethod
    def WrapAndSendPayload(cls, sock, payload):
        '''
        Send payload, true if success, false if failed
        '''
        try:
            # From SocketServer.py
            # wbufsize = 0, flush immediately
            wbufsize = -1
            # Convert
            socket_message = SocketMessage(payload)
            wfile = sock.makefile('wb', wbufsize)
            # Write the message
            wfile.write(struct.pack(cls.fmt, socket_message.magic))
            # Need to send the packed version
            # print 'Sent ', socket_message.magic

            wfile.write(struct.pack(cls.fmt, socket_message.payload_size))
            # print 'Sent ', socket_message.payload_size

            wfile.write(payload)
            # print 'Sent ', payload
            wfile.flush()
            wfile.close() # Close file object, not close the socket
            return True
        except Exception as e:
            _L.error('Fail to send message %s', e)
            return False


'''
BaseClient send message out and receiving message in a seperate thread.
After calling the `send` function, only True or False will be returned
to indicate whether the operation was successful.
If you are trying to send a request and get a response, consider using `Client` instead.
This class adds message framing on top of TCP
'''
class Client(object):
    '''
    Client can be used to send request to a game and get response
    Currently only one client is allowed at a time
    More clients will be rejected
    '''

    def __init__(self, endpoint):
        '''
        Parameters:
        endpoint: a tuple (ip, port)
        '''
        self.endpoint = endpoint
        self.sock = None # if socket == None, means client is not connected
        self.raw_message_regexp = re.compile(b'(\d{1,8}):(.*)') # A binary regexp
        self.message_id = 0
        self.wait_response = threading.Event()

    def send(self, message):
        ''' Send message out, return whether the message was successfully sent '''
        if self.isconnected():
            _L.debug('BaseClient: Send message %s', message)
            SocketMessage.WrapAndSendPayload(self.sock, message)
            return True
        else:
            _L.error('Fail to send message, client is not connected')
            return False

    def raw_message_handler(self, raw_message):
        match = self.raw_message_regexp.match(raw_message)

        if match:
            [message_id, message_body] = (int(match.group(1)), match.group(2)) # TODO: handle multiline response
            message_body = raw_message[len(match.group(1))+1:]
            # Convert to utf-8 if it's not a byte array (as is the case for images)
            try:
                message_body = message_body.decode('utf-8')
            except UnicodeDecodeError:
                pass
            # print 'Received message id %s' % message_id
            if message_id == self.message_id:
                return message_body
            else:
                assert(False)
        else:
            # Instead of just dropping this message, give a verbose notice
            _L.error('No message handler to handle message with length %d', len(raw_message))

    def connect(self, timeout = 1):
        '''
        Try to connect to server, return whether connection successful
        '''
        if self.isconnected():
            return True

        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            # Make the socket working in the blocking mode
            s.connect(self.endpoint)
            _L.debug('BaseClient: wait for connection confirm')

            message = SocketMessage.ReceivePayload(s)
            if message.startswith(b'connected'):
                _L.info('Got connection confirm: %s', repr(message))
                self.sock = s
                return True
            else:
                self.sock = None
                _L.error('Socket is created, but can not get connection confirm from %s, timeout after %.2f seconds', self.endpoint, timeout)
                return False

            # only assign self.socket to connected socket
            # so it is safe to use self.socket != None to check connection status
            # This does not neccessarily mean connection successful, might be closed by server
            # Unless explicitly to tell the server to accept new socket

        except Exception as e:
            _L.error('Can not connect to %s', str(self.endpoint))
            _L.error("Error %s", e)
            self.sock = None
            return False


    def isconnected(self):
        return self.sock is not None

    def disconnect(self):
        if self.isconnected():
            _L.debug("BaseClient, request disconnect from server in %s", threading.current_thread().name)

            self.sock.shutdown(socket.SHUT_RD)
            # Because socket is on read in __receiving thread, need to call shutdown to force it to close
            if self.sock: # This may also be set to None in the __receiving thread
                self.sock.close()
                self.sock = None
            time.sleep(0.1) # TODO, this is tricky

    def receive(self):
        '''
        Receive packages, Extract message from packages
        Call self.message_handler if got a message
        Also check whether client is still connected
        '''
        if self.isconnected():
            # Only this thread is allowed to read from socket, otherwise need lock to avoid competing
            message = SocketMessage.ReceivePayload(self.sock)
            _L.debug('Got server raw message with length %d', len(message))
            if not message:
                _L.debug('BaseClient: remote disconnected, no more message')
                self.sock = None

            return message

    def request_async(self, message):
        '''
        Send request without waiting for any reply
        '''
        if sys.version_info[0] == 3:
            if not isinstance(message, bytes):
                message = message.encode("utf-8")

        raw_message = b'%d:%s' % (self.message_id, message)
        if not self.send(raw_message):
            return None
        # self.message_id += 1 # Increment it only after the request/response cycle finished
        # Do not increase message_id, since no receive.
        return None

    def request_batch(self, batch):
        for i, message in enumerate(batch):
            if sys.version_info[0] == 3:
                if not isinstance(message, bytes):
                    message = message.encode("utf-8")

            raw_message = b'%d:%s' % (self.message_id + i, message)
            self.send(raw_message)
        # self.message_id += 1  # For the vbatch command

        batch_res = []
        for i in range(len(batch)):
            raw_message = self.receive() # block the receiving thread
            message = self.raw_message_handler(raw_message)
            self.message_id += 1 # Increment it only after the request/response cycle finished
            batch_res.append(message)
        
        return batch_res


    def request(self, message, timeout=5):
        """
        Send a request to server and wait util get a response from server or timeout.

        Parameters
        ----------
        cmd : str
            command to control the game. More info can be seen from http://docs.unrealcv.org/en/master/reference/commands.html

        Returns
        -------
        str
            plain text message from server

        Examples
        --------
        >>> client = Client('localhost', 9000)
        >>> client.connect()
        >>> response = client.request('vget /camera/0/view')
        """
        if type(message) is list:
            return self.request_batch(message)

        if sys.version_info[0] == 3:
            if not isinstance(message, bytes):
                message = message.encode("utf-8")

        raw_message = b'%d:%s' % (self.message_id, message)
        _L.debug('Request: %s', raw_message.decode("utf-8"))
        if not self.send(raw_message):
            return None

        raw_message = self.receive() # block the receiving thread
        message = self.raw_message_handler(raw_message)

        # Receive the message or timeout

        # Timeout is required
        # see: https://bugs.python.org/issue8844
        self.message_id += 1 # Increment it only after the request/response cycle finished

        return message

(HOST, PORT) = ('localhost', 9000)
client = Client((HOST, PORT))

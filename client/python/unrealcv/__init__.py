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

fmt = 'I'

class SocketMessage(object):
    '''
    Define the format of a message. This class is defined similar to the class FNFSMessageHeader in UnrealEngine4, but without CRC check.
    The magic number is from Unreal implementation
    See https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Source/Runtime/Sockets/Public/NetworkMessage.h
    '''
    magic = ctypes.c_uint32(0x9E2B83C1).value
    def __init__(self, payload):
        self.magic = SocketMessage.magic
        self.payload_size = ctypes.c_uint32(len(payload)).value

    @classmethod
    def ReceivePayload(cls, socket):
        '''
        Return only payload, not the raw message, None if failed.
        socket: a blocking socket for read data.
        '''
        # rbufsize = -1 # From SocketServer.py
        rbufsize = 0
        rfile = socket.makefile('rb', rbufsize)
        _L.debug('read raw_magic %s', threading.current_thread().name)
        try:
            raw_magic = rfile.read(4) # socket is disconnected or invalid
        except Exception as e:
            _L.debug('Fail to read raw_magic, %s', e)
            raw_magic = None

        _L.debug('read raw_magic %s done: %s', threading.current_thread().name, repr(raw_magic))
        if not raw_magic: # nothing to read
            # _L.debug('socket disconnect')
            return None
        # print 'Receive raw magic: %d, %s' % (len(raw_magic), raw_magic)
        magic = struct.unpack(fmt, raw_magic)[0] # 'I' means unsigned int
        # print 'Receive magic:', magic

        if magic != cls.magic:
            _L.error('Error: receive a malformat message, the message should start from a four bytes uint32 magic number')
            return None
            # The next time it will read four bytes again

        _L.debug('read payload')
        raw_payload_size = rfile.read(4)
        # print 'Receive raw payload size: %d, %s' % (len(raw_payload_size), raw_payload_size)
        payload_size = struct.unpack('I', raw_payload_size)[0]
        _L.debug('Receive payload size %d', payload_size)

        # if the message is incomplete, should wait until all the data received
        payload = b""
        remain_size = payload_size
        while remain_size > 0:
            data = rfile.read(remain_size)
            if not data:
                return None

            payload += data
            bytes_read = len(data) # len(data) is its string length, but we want length of bytes
            # print 'bytes_read %d, remain_size %d, read_str %s' % (bytes_read, remain_size, data)
            assert(bytes_read <= remain_size)
            remain_size -= bytes_read

        rfile.close()

        return payload

    @classmethod
    def WrapAndSendPayload(cls, socket, payload):
        '''
        Send payload, true if success, false if failed
        '''
        try:
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
        except Exception as e:
            _L.error('Fail to send message %s', e)
            return False


class BaseClient(object):
    '''
    BaseClient send message out and receiving message in a seperate thread.
    After calling the `send` function, only True or False will be returned
    to indicate whether the operation was successful.
    If you are trying to send a request and get a response, consider using `Client` instead.
    This class adds message framing on top of TCP
    '''
    def __init__(self, endpoint, raw_message_handler):
        '''
        Parameters:
        endpoint: a tuple (ip, port)
        message_handler: a function defined as `def message_handler(msg)` to handle incoming message, msg is a string
        '''
        self.endpoint = endpoint
        self.raw_message_handler = raw_message_handler
        self.socket = None # if socket == None, means client is not connected
        self.wait_connected = threading.Event()

        # Start a thread to get data from the socket
        receiving_thread = threading.Thread(target = self.__receiving)
        receiving_thread.setDaemon(1)
        receiving_thread.start()


    def connect(self, timeout = 1):
        '''
        Try to connect to server, return whether connection successful
        '''
        if self.isconnected():
            return True
            
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            s.connect(self.endpoint)
            self.socket = s
            _L.debug('BaseClient: wait for connection confirm')

            self.wait_connected.clear()
            isset = self.wait_connected.wait(timeout)
            assert(isset != None) # in python prior to 2.7 wait will return None
            if isset:
                return True
            else:
                self.socket = None
                _L.error('Socket is created, but can not get connection confirm from %s, timeout after %.2f seconds', self.endpoint, timeout)
                return False
            # only assign self.socket to connected socket
            # so it is safe to use self.socket != None to check connection status
            # This does not neccessarily mean connection successful, might be closed by server
            # Unless explicitly to tell the server to accept new socket

        except Exception as e:
            _L.error('Can not connect to %s', str(self.endpoint))
            _L.error("Error %s", e)
            self.socket = None
            return False


    def isconnected(self):
        return self.socket is not None

    def disconnect(self):
        if self.isconnected():
            _L.debug("BaseClient, request disconnect from server in %s", threading.current_thread().name)

            self.socket.shutdown(socket.SHUT_RD)
            # Because socket is on read in __receiving thread, need to call shutdown to force it to close
            if self.socket: # This may also be set to None in the __receiving thread
                self.socket.close()
                self.socket = None
            time.sleep(0.1) # TODO, this is tricky

    def __receiving(self):
        '''
        Receive packages, Extract message from packages
        Call self.message_handler if got a message
        Also check whether client is still connected
        '''
        _L.debug('BaseClient start receiving in %s', threading.current_thread().name)
        while True:
            if self.isconnected():
                # Only this thread is allowed to read from socket, otherwise need lock to avoid competing
                message = SocketMessage.ReceivePayload(self.socket)
                _L.debug('Got server raw message %s', message)
                if not message:
                    _L.debug('BaseClient: remote disconnected, no more message')
                    self.socket = None
                    continue

                if message.startswith(b'connected'):
                    _L.info('Got connection confirm: %s', repr(message))
                    self.wait_connected.set()
                    # self.wait_connected.clear()
                    continue

                if self.raw_message_handler:
                    self.raw_message_handler(message) # will block this thread
                else:
                    _L.error('No message handler for raw message %s', message)

    def send(self, message):
        '''
        Send message out, return whether the message was successfully sent
        '''
        if self.isconnected():
            _L.debug('BaseClient: Send message %s', self.socket)
            SocketMessage.WrapAndSendPayload(self.socket, message)
            return True
        else:
            _L.error('Fail to send message, client is not connected')
            return False


class Client(object):
    '''
    Client can be used to send request to a game and get response
    Currently only one client is allowed at a time
    More clients will be rejected
    '''
    def __raw_message_handler(self, raw_message):
        # print 'Waiting for message id %d' % self.message_id
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
                self.response = message_body
                self.wait_response.set()
            else:
                assert(False)
        else:
            if self.message_handler:
                def do_callback():
                    self.message_handler(raw_message)
                self.queue.put(do_callback)
            else:
                # Instead of just dropping this message, give a verbose notice
                _L.error('No message handler to handle message %s', raw_message)

    def __init__(self, endpoint, message_handler=None):
        self.raw_message_regexp = re.compile(b'(\d{1,8}):(.*)')
        self.message_client = BaseClient(endpoint, self.__raw_message_handler)
        self.message_handler = message_handler
        self.message_id = 0
        self.wait_response = threading.Event()
        self.response = ''

        self.isconnected = self.message_client.isconnected
        self.connect = self.message_client.connect
        self.disconnect = self.message_client.disconnect

        self.queue = Queue()
        self.main_thread = threading.Thread(target = self.worker)
        self.main_thread.setDaemon(1)
        self.main_thread.start()

    def worker(self):
        while True:
            task = self.queue.get()
            task()
            self.queue.task_done()

    def request(self, message, timeout=5):
        # docstring in numpy style
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
        if sys.version_info[0] == 3:
          if not isinstance(message, bytes):
            message = message.encode("utf-8")
        def do_request():
            raw_message = b'%d:%s' % (self.message_id, message)
            _L.debug('Request: %s', raw_message.decode("utf-8"))
            if not self.message_client.send(raw_message):
                return None

        # request can only be sent in the main thread, do not support multi-thread submitting request together
        if threading.current_thread().name == self.main_thread.name:
            do_request()
        else:
            self.queue.put(do_request)

        # Timeout is required
        # see: https://bugs.python.org/issue8844
        self.wait_response.clear() # This is important
        isset = self.wait_response.wait(timeout)
        self.message_id += 1 # Increment it only after the request/response cycle finished

        assert(isset != None) # only python prior to 2.7 will return None
        if isset:
            return self.response
        else:
            _L.error('Can not receive a response from server, timeout after %.2f seconds', timeout)
            return None

(HOST, PORT) = ('localhost', 9000)
client = Client((HOST, PORT), None)

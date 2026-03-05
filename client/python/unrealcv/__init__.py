import ctypes
import logging
import re
import socket
import struct
import sys
import threading
import time
import os
from .api import *
from .automation import *
from .launcher import *
from queue import Empty, SimpleQueue


_L = logging.getLogger(__name__)
if not any(isinstance(handler, logging.NullHandler) for handler in _L.handlers):
    _L.addHandler(
        logging.NullHandler()
    )  # Library best practice: let consumers configure logging

__version__ = "1.1.7"  # add async request, IPC on linux >= 1.0.0


class SocketMessage:
    """
    Define the format of a message. This class is defined similar to the class FNFSMessageHeader in UnrealEngine4, but without CRC check.
    The magic number is from Unreal implementation
    See https://github.com/EpicGames/UnrealEngine/blob/dff3c48be101bb9f84633a733ef79c91c38d9542/Engine/Source/Runtime/Sockets/Public/NetworkMessage.h
    """

    magic = ctypes.c_uint32(0x9E2B83C1).value
    fmt = "I"

    def __init__(self, payload):
        self.payload_size = ctypes.c_uint32(len(payload)).value

    @classmethod
    def _recv_exact(cls, sock, nbytes):
        """
        Read exactly nbytes from sock.

        Returns bytes on success, None on clean disconnect (recv returns 0);
        raises on socket error.
        """
        buf = bytearray(nbytes)
        view = memoryview(buf)
        offset = 0
        while offset < nbytes:
            read_size = sock.recv_into(view[offset:], nbytes - offset)
            if read_size == 0:
                return None
            offset += read_size
        return bytes(buf)

    @classmethod
    def ReceivePayload(cls, sock):
        """
        Return only payload, not the raw message, None if failed.
        sock: a blocking socket for read data.
        """
        try:
            raw_magic = cls._recv_exact(sock, 4)
        except Exception as e:
            _L.debug('Fail to read raw_magic, exception: "%s"', e)
            raw_magic = None

        if not raw_magic:
            # _L.debug('socket disconnect by server')
            return None

        magic = struct.unpack(cls.fmt, raw_magic)[0]  # 'I' means unsigned int
        if magic != cls.magic:
            _L.error(
                "Received malformed message: expected magic 0x%08X, got 0x%08X",
                cls.magic,
                magic,
            )
            return None
            # The next time it will read four bytes again

        raw_payload_size = cls._recv_exact(sock, 4)
        if raw_payload_size is None:
            _L.warning("Socket disconnected by server (payload size)")
            return None
        payload_size = struct.unpack("I", raw_payload_size)[0]

        payload = cls._recv_exact(sock, payload_size)
        if payload is None:
            _L.warning("Socket disconnected by server (payload body)")
            return None

        return payload

    @classmethod
    def WrapAndSendPayload(cls, sock, payload):
        """
        Send payload, true if success, false if failed
        """
        try:
            header = struct.pack(cls.fmt + cls.fmt, cls.magic, len(payload))
            sock.sendall(header)
            sock.sendall(payload)
            return True
        except Exception as e:
            _L.error("Fail to send message %s", e)
            return False


"""
BaseClient send message out and receiving message in a seperate thread.
After calling the `send` function, only True or False will be returned
to indicate whether the operation was successful.
If you are trying to send a request and get a response, consider using `Client` instead.
This class adds message framing on top of TCP
"""


class Client:
    """
    Client can be used to send request to a game and get response
    Currently only one client is allowed at a time
    More clients will be rejected
    """

    SOCKET_TIMEOUT = 60
    RECONNECT_ATTEMPTS = 5
    RECONNECT_BASE_DELAY = 0.5
    RECONNECT_MAX_DELAY = 8.0

    def __init__(self, endpoint, type="inet"):
        """
        Parameters:
        endpoint: a tuple (ip, port)
        type: unix or inet
        """
        self.endpoint = endpoint
        self.sock = None  # if socket == None, means client is not connected
        self.raw_message_regexp = re.compile(rb"(\d{1,}):(.*)")  # A binary regexp
        # self.message_id = 0
        self.wait_response = threading.Event()
        self.send_message_id = 0
        self.recv_message_id = 0
        self.recv_num_q = SimpleQueue()  # inf
        self.recv_data_q = SimpleQueue()  # inf
        self.type = type
        self._sock_lock = threading.Lock()
        self.t = None

    def _next_message_id(self):
        with self._sock_lock:
            message_id = self.send_message_id
            self.send_message_id += 1
            return message_id

    def send(self, message):
        """Send message out, return whether the message was successfully sent"""
        with self._sock_lock:
            if self.isconnected():
                _L.debug("BaseClient: Send message %s", message)
                return SocketMessage.WrapAndSendPayload(self.sock, message)
            else:
                _L.error("Fail to send message, client is not connected")
                return False

    def raw_message_handler(self, raw_message):
        match = self.raw_message_regexp.match(raw_message)

        if match:
            [message_id, message_body] = (
                int(match.group(1)),
                match.group(2),
            )
            message_body = raw_message[len(match.group(1)) + 1 :]
            # Convert to utf-8 if it's not a byte array (as is the case for images)
            try:
                message_body = message_body.decode("utf-8")
            except UnicodeDecodeError:
                pass
            # print 'Received message id %s' % message_id
            if message_id == self.recv_message_id:
                return message_body
            else:
                assert False, (
                    f"this_msg_id: {message_id}; record_msg_id: {self.recv_message_id}"
                )
        else:
            # Instead of just dropping this message, give a verbose notice
            _L.error(
                "No message handler to handle message with length %d", len(raw_message)
            )

    def connect(self, timeout=1, start_receive_thread=True):
        """
        Try to connect to server, return whether connection successful
        """
        if self.isconnected():
            return True

        s = None
        try:
            if self.type == "unix":
                _L.info("Using UDS socket")
                s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            elif self.type == "inet":
                _L.info("Using ip-port socket")
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            else:
                raise NotImplementedError
            s.settimeout(self.SOCKET_TIMEOUT if timeout is None else timeout)
            s.connect(self.endpoint)
            s.settimeout(self.SOCKET_TIMEOUT)
            with self._sock_lock:
                self.sock = s
            _L.debug("BaseClient: wait for connection confirm")

            message = SocketMessage.ReceivePayload(self.sock)
            if message is not None:
                if message.startswith(b"connected"):
                    _L.info("Got connection confirm: %s", repr(message))

                    if start_receive_thread and (
                        self.t is None or not self.t.is_alive()
                    ):
                        self.t = threading.Thread(
                            target=self.receive_loop_queue,
                            daemon=True,
                        )
                        self.t.start()

                    return True

            # self.sock = None
            self.disconnect()
            _L.error(
                "Socket is created, but can not get connection confirm from %s. Disconnect!",
                self.endpoint,
            )
            return False

            # only assign self.socket to connected socket
            # so it is safe to use self.socket != None to check connection status
            # This does not neccessarily mean connection successful, might be closed by server
            # Unless explicitly to tell the server to accept new socket

        except Exception as e:
            _L.error("Can not connect to %s", str(self.endpoint))
            _L.error("Error %s", e)
            if s is not None:
                try:
                    s.close()
                except OSError:
                    pass
            self.disconnect()
            # self.sock = None
            return False

    def isconnected(self):
        """Check whether client is connected to server"""
        return self.sock is not None

    def disconnect(self):
        """Disconnect from server"""
        with self._sock_lock:
            if self.sock is not None:
                _L.debug(
                    "BaseClient, request disconnect from server in %s",
                    threading.current_thread().name,
                )
                try:
                    self.sock.shutdown(socket.SHUT_RD)
                except OSError:
                    pass
                try:
                    self.sock.close()
                except OSError:
                    pass
                self.sock = None

        if self.t is not None and self.t.is_alive():
            self.recv_num_q.put(None)
            if threading.current_thread() is not self.t:
                self.t.join(timeout=5)

    def receive(self):
        """
        Receive packages, Extract message from packages
        Call self.message_handler if got a message
        Also check whether client is still connected
        """
        with self._sock_lock:
            sock = self.sock

        if sock is not None:
            message = SocketMessage.ReceivePayload(sock)

            # message may be None here
            # _L.debug('Got server raw message with length %d', len(message))

            if not message:
                _L.warning("BaseClient: remote disconnected, no more message")
                self.disconnect()

                delay = self.RECONNECT_BASE_DELAY
                for attempt in range(1, self.RECONNECT_ATTEMPTS + 1):
                    _L.info(
                        "Reconnect attempt %d/%d (delay %.1fs)",
                        attempt,
                        self.RECONNECT_ATTEMPTS,
                        delay,
                    )
                    if self.connect(
                        timeout=self.SOCKET_TIMEOUT, start_receive_thread=False
                    ):
                        _L.info("Reconnect succeeded on attempt %d", attempt)
                        return self.receive()
                    time.sleep(delay)
                    delay = min(delay * 2, self.RECONNECT_MAX_DELAY)

                error = ConnectionError(
                    f"Failed to reconnect to {self.endpoint} after "
                    f"{self.RECONNECT_ATTEMPTS} attempts"
                )
                self.recv_data_q.put(error)
                raise error

            return message

    def receive_loop_queue(self):
        while True:
            num = self.recv_num_q.get()

            # quit signal
            if num is None:
                break

            if num < 0:
                # need results
                for _ in range(-num):
                    try:
                        raw_message = self.receive()
                    except Exception as exception:
                        self.recv_data_q.put(exception)
                        break
                    if raw_message is None:
                        self.recv_data_q.put(
                            ConnectionError("Disconnected before receiving response")
                        )
                        break
                    message = self.raw_message_handler(raw_message)
                    self.recv_message_id += (
                        1  # Increment it only after the request/response cycle finished
                    )
                    self.recv_data_q.put(message)
            else:
                # do not need results
                for _ in range(num):
                    try:
                        raw_message = self.receive()
                    except Exception:
                        break
                    self.recv_message_id += 1

    def request_async(self, message):
        """
        Send request without waiting for any reply
        """
        if type(message) is list:
            return self.request_batch_async(message)

        if sys.version_info[0] == 3:
            if not isinstance(message, bytes):
                message = message.encode("utf-8")

        message_id = self._next_message_id()
        raw_message = b"%d:%s" % (message_id, message)
        if not self.send(raw_message):
            raise ConnectionError("Failed to send because socket is closed")

        self.recv_num_q.put(1)
        # self.message_id += 1
        return None

    def request_batch_async(self, batch):
        """
        Send a batch of requests to server without waiting for any reply.

        batch : list
            a list of requests, each request is a string, such as ['command1', 'command2', ...]

        Returns
        -------
        None
        """
        for message in batch:
            if sys.version_info[0] == 3:
                if not isinstance(message, bytes):
                    message = message.encode("utf-8")

            message_id = self._next_message_id()
            raw_message = b"%d:%s" % (message_id, message)
            if not self.send(raw_message):
                raise ConnectionError("Failed to send because socket is closed")

        self.recv_num_q.put(len(batch))
        return None

    def request_batch(self, batch):
        """
        Send a batch of requests to server and wait util get all responses from server.
        Parameters
        ----------
        batch : list
            a list of requests, each request is a string, such as ['command1', 'command2', ...]
        Returns
        -------
        list
            a list of responses, such as ['response1', 'response2', ...]

        Examples
        --------
        >>> client.request_batch(['vget /camera/0/location', 'vget /camera/0/rotation'])
        ['100.0 -100.0 100.0', '0.0 0.0 0.0']
        """
        for message in batch:
            if sys.version_info[0] == 3:
                if not isinstance(message, bytes):
                    message = message.encode("utf-8")

            message_id = self._next_message_id()
            raw_message = b"%d:%s" % (message_id, message)
            if not self.send(raw_message):
                raise ConnectionError("Failed to send because socket is closed")

        self.recv_num_q.put(-len(batch))  # negative number indicates need results

        batch_res = []
        for i in range(len(batch)):
            message = self.recv_data_q.get()
            if isinstance(message, Exception):
                raise message
            batch_res.append(message)

        return batch_res

    def request(self, message, timeout=5):
        """
        Send a request to server and wait util get a response from server or timeout.

        Parameters
        ----------
        message : str or list
            UnrealCV command to interact with the game.
            When message is a list of commands, the commands will be sent in batch.
            More info can be seen from http://docs.unrealcv.org/en/latest/reference/commands.html

        timeout : int
            when timeout is larger than 0, the request will be sent synchronously, and the response will be returned
            when timeout is -1, the request will be sent asynchronously, and no response will be returned
        Returns
        -------
        str
            plain text message from server

        Examples
        --------
        >>> client = Client('localhost', 9000)
        >>> client.connect()
        >>> client.request('vget /camera/0/location')
        '100.0 -100.0 100.0'
        >>> # timeout -1 means async request, no response will be returned
        >>> client.request('vset /camera/0/location 100 100 100', -1)
        """

        if timeout < 0:  # async
            if type(message) is list:
                self.request_batch_async(message)
            else:
                self.request_async(message)
            return True

        if type(message) is list:
            return self.request_batch(message)

        if sys.version_info[0] == 3:
            if not isinstance(message, bytes):
                message = message.encode("utf-8")

        message_id = self._next_message_id()
        raw_message = b"%d:%s" % (message_id, message)
        # _L.debug('Request: %s', raw_message.decode("utf-8"))
        if not self.send(raw_message):
            raise ConnectionError("Failed to send because socket is closed")

        self.recv_num_q.put(-1)  # negative number indicates need results
        try:
            message = self.recv_data_q.get(timeout=timeout)
        except Empty as exception:
            raise TimeoutError(
                f"Request timed out after {timeout} seconds"
            ) from exception

        if isinstance(message, Exception):
            raise message

        return message


# To use IPC on Unix, set this path to: /tmp/unrealcv_{portnum}.socket
# Your executable will create this file on startup.

# print('=> Info: Use inet client...')
# (HOST, PORT) = ('localhost', 9000)
# client = Client((HOST, PORT), 'inet')

# unix_socket_path = '/tmp/unrealcv_9000.socket' # for example
# if 'linux' in sys.platform and unix_socket_path is not None and os.path.exists(unix_socket_path):
#     print('=> Info: Use UDS client...')
#     client = Client(unix_socket_path, 'unix')

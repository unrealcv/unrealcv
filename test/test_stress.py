'''
Stress test to measure performance and whether stable during connection lost
Also check correctness for high throughput case
'''
from test_common import *
import socket, time

class Timer(object):
    def __init__(self, name=None):
        self.name = name

    def __enter__(self):
        self.tstart = time.time()

    def __exit__(self, type, value, traceback):
        if self.name:
            print '[%s]' % self.name,
        print 'Elapsed: %s' % (time.time() - self.tstart)

if __name__ == '__main__':
    client = Client(HOST, PORT)
    print 'Try to connect (%s, %s)' % (HOST, PORT)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    raw_payload = 'hello' * 5000
    with Timer():
        for i in range(100):
            payload = '%d:%s' % (i, raw_payload)
            print i, 'Send a payload, size %d' % len(payload)
            SocketMessage.WrapAndSendPayload(s, payload)
            # Most of the time  broken pipe means remote connection is closed

            # print 'Wait for a payload'
            # reply_payload = SocketMessage.ReceivePayload(s)
            # # Make sure the reciving thread not taking the message before here.

            # print reply_payload[0:10]
        s.close()


# TODO: test malformat message

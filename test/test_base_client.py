# One thread for sending message and one thread for receiving.
# Do not support request and response

from test_common import *
import time

def RawMessageHandler(message):
    print message

if __name__ == '__main__':
    base_client = BaseClient((HOST, PORT), RawMessageHandler)
    sleep_time = 1

    for i in range(100):
        base_client.send('%d:test message' % i)
        time.sleep(sleep_time)

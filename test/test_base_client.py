# One thread for sending message and one thread for receiving.
# Do not support request and response

from test_common import *

def RawMessageHandler(message):
    print message

if __name__ == '__main__':
    base_client = BaseClient((HOST, PORT), RawMessageHandler)

    for i in range(100):
        client.send('test message')
        time.sleep(1)

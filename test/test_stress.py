from test_common import *
import time, threading


def MessageHandler(message):
    print message

if __name__ == '__main__':
    client = Client((HOST, PORT), MessageHandler)
    # Do connect automatically
    for i in range(100):
        reply = client.send('vget /mode')
        # Need to lock until I got a reply
        # print reply
        time.sleep(1)

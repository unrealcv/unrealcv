# A script to test ue4cv client
import sys
sys.path.append('.')

import ue4cv as U
import argparse
import time
import logging as L
L.basicConfig(level = L.DEBUG)

def on_received(data):
    message = repr(data)
    print 'IC: %s' % message

def on_connected():
    while 1:
        message = raw_input('Command to server: ')
        # client.send_raw(message)
        client.send(message)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', type=int, default=8000)
    parser.add_argument('--ip', default='127.0.0.1')
    args = parser.parse_args()

    client = U.Client(args.ip, args.port) # Client implementation should be very stable, it is hard to diagnosis
    client.set_connectd_handler(on_connected)
    client.set_received_handler(on_received)
    client.start_connection()

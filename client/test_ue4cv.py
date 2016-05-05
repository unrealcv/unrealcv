# A script to test ue4cv client
import ue4cv as U
import argparse
import time

def on_received(data):
    message = repr(data)
    print message

def on_connected():
    client.send('vget /object') # TODO: How to end a message??
    # Wait data response
    time.sleep(0.1)
    client.send('vget /camera/0/image')
    time.sleep(0.1)
    client.send('vget /camera/0/position')
    time.sleep(0.1)
    client.send('vset /mode/depth')

    # A list of location and rotation
    # Only valid location and rotation are acceptable
    # camera_confs = []
    # for _ in range(1000): # Randomly get 1000 camera configuration
    #     position = (0, 0, 0)
    #     rotation = (0, 0, 0)
    #     c = {'position': position, 'rotation': rotation}
    #     camera_confs.append(c)

    # for c in camera_confs:
    #     render_frame(c)

def render_frame(c):
    client.send('vset /camera/0/rotation %s' % str(c['rotation'])) # Check whether the response is successful and wait for message
    client.send('vset /camera/0/position %s' % str(c['position']))
    client.send('vget /camera/0/image') # Do I set filename?

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', type=int, default=8000)
    args = parser.parse_args()

    client = U.client('127.0.0.1', args.port) # Client implementation should be very stable, it is hard to diagnosis
    client.set_connectd_handler(on_connected)
    client.set_received_handler(on_received)
    client.connect()

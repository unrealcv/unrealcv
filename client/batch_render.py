import ue4cv as U # Or UnrealCV ?
import argparse
# I can define the sequence in here or define it in the mathinee movie editor.
def received_handler(message):
    print message

client = None # Make this global
def send_message(command):
    print command
    client.send(command)

def render_frame(cam_location):
    rot = ' '.join([str(v) for v in cam_location['rot']])
    loc = ' '.join([str(v) for v in cam_location['loc']])
    send_message('vset /camera/0/rotation %s' % rot)
    # Check whether the response is successful and wait for message
    send_message('vset /camera/0/location %s' % loc)
    send_message('vget /camera/0/image') # Do I set filename?

def set_mode(mode):
    send_message('vset /mode/%s' % mode)

def exec_rendering():
    print 'exec_rendering'
    render_mode = ['depth', 'object', 'lit', 'unlit']
    with open('./recorder.txt') as f:
        lines = f.readlines()

    cam_locations = []
    for pos_index in range(0, len(lines), 3):
        filename = lines[pos_index + 0].strip()
        loc = [float(v) for v in lines[pos_index + 1].split(' ')]
        rot = [float(v) for v in lines[pos_index + 2].split(' ')]
        cam_locations.append({'filename':filename, 'loc':loc, 'rot':rot})

    for mode in render_mode:
        set_mode(mode)
        for cam_location in cam_locations:
            print cam_location
            render_frame(cam_location) # Use location, do not use position

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', type=int, default=8000)
    parser.add_argument('--ip', default='127.0.0.1')
    args = parser.parse_args()

    client = U.Client(args.ip, args.port) # Client implementation should be very stable, it is hard to diagnosis
    client.set_connectd_handler(exec_rendering)
    client.set_received_handler(received_handler)
    client.start_connection()

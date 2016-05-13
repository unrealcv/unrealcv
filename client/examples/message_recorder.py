# Record all the messages (not raw message) from the servers
import argparse
import ue4cv as U
import logging as L
L.basicConfig(level=L.INFO)

def save_to_file(filename):
    f = open(filename, 'w')
    def on_received(message):
        L.info(message)
        f.write(message)
    return on_received


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', type=int, default=8000)
    parser.add_argument('--ip', default='127.0.0.1')
    parser.add_argument('--filename', default='recorder.txt')
    args = parser.parse_args()

    client = U.client(args.ip, args.port)

    client.set_received_handler(save_to_file(args.filename))

    client.connect()

import sys, argparse, time
sys.path.append('..')
from ue4cv import *

HOST, PORT = "localhost", 9000
# HOST, PORT = "xd", 9000

def message_handler(message):
    print 'Message from server: %s' % repr(message) # Need to show \x00 also
    f.write(message + '\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('logfile')
    args = parser.parse_args()

    f = open(args.logfile, 'w') # TODO: Make sure the message has been flushed
    client = Client((HOST, PORT), message_handler)

    time.sleep(60 * 60)

from test_common import *
import time

test_commands = [
    'vget /mode',
    'vget /objects',
    'vset /mode/depth',
    'vget /camera/0/location',
    'vget /camera/0/rotation',
    'vget /camera/0/image', # TODO: The regexp in server needs to do exact match
    'vget /camera/0/depth',
    'vget /camera/0/object_mask',
    'vget /camera/0/normal',
    'vset /mode/lit',
    # 'vrun ls',
    # 'vget /object/_/name', this needs to be run on game thread
]

def run_commands():
    sleep_time = 1
    for i in range(100):
        print '-' * 70
        cmd = test_commands[i % len(test_commands)]
        print 'Cmd: %s' % cmd
        response = client.request(cmd)
        print 'Response: %s' % response
        # Need to lock until I got a reply
        # print reply
        time.sleep(sleep_time)


def MessageHandler(message):
    print 'Message from server: %s' % message

if __name__ == '__main__':
    client = Client((HOST, PORT), MessageHandler)

    # run_commands()
    time.sleep(60 * 60)

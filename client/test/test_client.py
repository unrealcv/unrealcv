from test_common import *
import time, os
has_plt = False
try:
    import matplotlib.pyplot as plt
    has_plt = True
except:
    pass

def skip(response):
    return True
    pass

def justcreated(filename, timelimit=1):
    # Make sure the file is just created
    now = time.time()
    t = os.path.getctime(filename)
    delta = now - t
    assert(delta > 0) # file is created before this function
    assert(delta < timelimit) # file is just created
    return True


def ispng(response):
    assert(response.endswith('.png'))
    # Make sure file exsit
    assert(os.path.isfile(response))

    # Make sure file readable
    if has_plt:
        im = plt.imread(response)

    # Make sure file is just created
    justcreated(response)
    return True

test_commands = [
    ['vset /mode/depth', 'ok'],
    ['vget /mode', 'depth'],
    ['vget /objects', skip],
    ['vget /camera/0/location', skip],
    ['vset /camera/0/rotation 0.0 0.0 0.0', 'ok'],
    ['vget /camera/0/rotation', '0.000 0.000 0.000'], # TODO: Change the Uri handle to exact map
    ['vget /camera/0/image', ispng], # TODO: The regexp in server needs to do exact match
    # Check the async operation, make sure the task is already completed.
    ['vget /camera/0/depth', ispng],
    ['vget /camera/0/object_mask', ispng],
    ['vget /camera/0/normal', skip],
    ['vset /mode/lit', 'ok'],
    # 'vrun ls',
    # 'vget /object/_/name', this needs to be run on game thread
    # TODO: add a test for object location
    # Check different view mode
    ['vget /camera/0/normal', ispng],
    ['vget /camera/0/base_color', ispng],
    ['vget /camera/0/depth', ispng],
    ['vget /camera/0/lit', ispng],
    ['vget /camera/0/object_mask', ispng],
    ['vget /camera/0/view', ispng],
]

def test_some_locations(client):
    test_camera_locations = get_filelist('./correctness_test/camera_info_basename.txt')
    for camera_location in test_camera_locations:
        res = client.request('vset /camera/0/location %s' % ' '.join([str(v) for v in camera_location[1]]))
        assert(res == 'ok')
        res = client.request('vset /camera/0/rotation %s' % ' '.join([str(v) for v in camera_location[2]]))
        assert(res == 'ok')

        # modes = ['normal', 'base_color', 'depth', 'lit', 'unlit', 'object_mask']
        modes = ['normal', 'base_color', 'depth', 'lit', 'unlit', 'object_mask'] # Buffer Visualization
        for mode in modes: # TODO: shuffle the order of modes
            time.sleep(0)
            # Make sure the result is consistent
            res = client.request('vget /camera/0/%s' % mode)
            print res
            # res = client.request('vget /camera/0/%s' % mode)
            # print res
            # res = client.request('vget /camera/0/%s' % mode)
            # print res
            assert(ispng(res))

def run_commands(client):
    sleep_time = 0
    for i in range(100):
        print '-' * 70
        task = test_commands[i % len(test_commands)]
        cmd = task[0]
        expect = task[1]

        print 'Cmd: %s' % cmd
        response = client.request(cmd)
        print 'Response: %s' % repr(response)
        # Need to lock until I got a reply
        # print reply

        if isinstance(expect, str):
            assert(response == expect)
        else:
            expect(response)

        time.sleep(sleep_time)


def MessageHandler(message):
    print 'Message from server: %s' % repr(message) # Need to show \x00 also

if __name__ == '__main__':
    client = Client((HOST, PORT), MessageHandler)

    run_commands(client)
    # test_some_locations(client)

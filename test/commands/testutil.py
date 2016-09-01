import os, time, logging
_L = logging.getLogger(__name__)
_L.setLevel(logging.DEBUG)

def validate_format(response):
    valid = True
    if not response:
        return False
    valid &= (response == response.strip())
    return valid

# TODO: Check the async operation, make sure the task is already completed.
def justcreated(filename, timelimit=1):
    flag = True

    flag &= validate_format(filename)

    # Make sure the file is just created
    now = time.time()
    t = os.path.getctime(filename)
    delta = now - t
    flag &= (delta > 0) # file is created before this function
    flag &= (delta < timelimit) # file is just created
    return flag

def ispng(response):
    flag = True
    flag &= validate_format(response)
    flag &= response.endswith('.png')
    # Make sure file exsit
    flag &= os.path.isfile(response)

    # Make sure file is just created
    flag &= justcreated(response)
    return flag

def skip(response):
    flag = True
    flag &= validate_format(response)
    return flag

def run_tasks(testcase, client, tasks):
    '''
    testcase: test case instance
    client: unrealcv client
    tasks: a list of tasks to run
    '''
    for task in tasks:
        cmd = task[0]
        expect = task[1]

        _L.debug('Cmd: %s' % cmd)
        response = client.request(cmd)
        # if response == None:
        #     testcase.assertTrue(False, 'Can not connect to UnrealCV server')
        #     return

        _L.debug('Response: %s' % repr(response))
        # Need to lock until I got a reply
        # print reply

        error_message = 'cmd: %s, expect: %s, response %s' % (cmd, str(expect), response)
        if expect == None or isinstance(expect, str):
            testcase.assertEqual(response, expect, error_message)
        else:
            testcase.assertTrue(expect(response), error_message)

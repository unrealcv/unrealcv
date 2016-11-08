'''
Run python test.py to test unrealcv
'''
import unittest, time, sys, argparse, threading
from common_conf import *

def run_full_test():
    run_travis_test()
    run_dev_test()

def run_dev_test():
    sys.path.append('./test/testsuite')
    suites = []
    import test_connection, test_fps, test_camera
    suites.append(test_connection.test_suite)
    suites.append(test_camera.test_suite)
    # suites.append(test_fps.test_suite)

    suite_obj = unittest.TestSuite(suites)
    ret = not unittest.TextTestRunner(verbosity = 2, failfast = True).run(suite_obj).wasSuccessful()
    return ret

def run_travis_test():
    sys.path.append('./test/ipc')
    from test_dev_server import TestDevServer
    from test_client import TestUE4CVClient
    load = unittest.TestLoader().loadTestsFromTestCase

    suites = []
    s = load(TestDevServer); suites.append(s)
    s = load(TestUE4CVClient); suites.append(s)
    suite_obj = unittest.TestSuite(suites)

    ret = not unittest.TextTestRunner(verbosity = 2).run(suite_obj).wasSuccessful()

    return ret

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--travis', action='store_true') # Only run test availabe to travis CI
    parser.add_argument('--dev', action='store_true') # Only run test availabe to travis CI

    args = parser.parse_args()
    if args.travis:
        ret = run_travis_test()
    if args.dev:
        ret = run_dev_test()
    # ret = run_full_test()

    sys.exit(ret)

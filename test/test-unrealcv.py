'''
Run python test.py to test unrealcv
'''
import unittest, time, sys, argparse, threading
from common_conf import *

def run_full_test():
    pass

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

    args = parser.parse_args()
    if args.travis:
        ret = run_travis_test()
    else:
        ret = run_full_test()

    sys.exit(ret)

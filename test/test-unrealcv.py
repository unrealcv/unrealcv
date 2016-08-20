# TODO: Test robustness, test speed
import unittest, time, sys, argparse, threading
sys.path.append('ipc')
from common_conf import *
from test_dev_server import TestDevServer
from test_client import TestUE4CVClient

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--travis', action='store_true') # Only run test availabe to travis CI

    args = parser.parse_args()
    suites = []

    load = unittest.TestLoader().loadTestsFromTestCase
    s = load(TestDevServer); suites.append(s)
    s = load(TestUE4CVClient); suites.append(s)

    suite_obj = unittest.TestSuite(suites)
    ret = not unittest.TextTestRunner(verbosity = 2).run(suite_obj).wasSuccessful()
    sys.exit(ret)

# TODO: Test robustness, test speed
import unittest, time, sys, argparse, threading
from common_conf import *
from test_dev_server import EchoServer, MessageServer, TestDevServer
from test_client import TestUE4CVClient
from test_commands import TestCommands
from test_realistic_rendering import TestRealisticRendering

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--travis', action='store_true') # Only run test availabe to travis CI

    args = parser.parse_args()
    suites = []

    load = unittest.TestLoader().loadTestsFromTestCase
    s = load(TestDevServer); suites.append(s)
    s = load(TestUE4CVClient); suites.append(s)
    if not args.travis:
        s = load(TestCommands); suites.append(s)
        s = load(TestRealisticRendering); suites.append(s)

    suite_obj = unittest.TestSuite(suites)
    ret = not unittest.TextTestRunner(verbosity = 2).run(suite_obj).wasSuccessful()
    sys.exit(ret)

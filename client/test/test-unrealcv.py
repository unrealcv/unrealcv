# TODO: Test robustness, test speed
import unittest, time, sys
from test_common import *
from test_server import EchoServer, MessageServer
import argparse
import threading
from test_server import TestMessageServer
from test_client import TestClientWithDummyServer
from test_commands import TestCommands
from test_realistic_rendering import TestRealisticRendering

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--travis', action='store_true') # Only run test availabe to travis CI

    args = parser.parse_args()
    suites = []

    load = unittest.TestLoader().loadTestsFromTestCase
    s = load(TestMessageServer); suites.append(s)
    s = load(TestClientWithDummyServer); suites.append(s)
    if not args.travis:
        s = load(TestCommands); suites.append(s)
        s = load(TestRealisticRendering); suites.append(s)

    suite_obj = unittest.TestSuite(suites)
    unittest.TextTestRunner(verbosity = 2).run(suite_obj)

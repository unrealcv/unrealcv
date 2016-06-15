'''
Define configuration for testing
Common utility functions
'''
import sys, os
libpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
sys.path.append(libpath)

HOST, PORT = "localhost", 9000
# HOST, PORT = "xd", 9000

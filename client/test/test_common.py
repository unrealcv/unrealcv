'''
Define configuration for testing
'''
import sys, os
libpath = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')
sys.path.append(libpath)
from ue4cv import *


HOST, PORT = "localhost", 9000
# HOST, PORT = "xd", 9000

def get_filelist(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    # Parse camera location and rotation from file
    camera_pos = []
    for line_id in range(len(lines)):
        line = lines[line_id].strip() # Remove \n at the end
        if line_id % 3 == 0: # filename
            filename = line
        elif line_id % 3 == 1: # location
            location = [float(v) for v in line.split(' ')]
        elif line_id % 3 == 2: # Rotation
            rotation = [float(v) for v in line.split(' ')]
            camera_pos.append((filename, location, rotation))
    return camera_pos

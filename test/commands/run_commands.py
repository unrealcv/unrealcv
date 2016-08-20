# Load all commands from ls.py
# Iterate over all commands, make sure no command return error.
import sys, time
sys.path.append('../../')
import ue4cv

ue4cv.client.connect()

res = ue4cv.client.request('vget /unrealcv/help')
lines = res.split('\n')
commands = lines[0::2]
helps = lines[1::2]

for cmd in commands:
    print cmd
    res = ue4cv.client.request(cmd)
    print res

time.sleep(5)

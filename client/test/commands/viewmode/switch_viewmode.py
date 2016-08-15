import time, sys, shutil
sys.path.append('../../..')
# Use timer to check time.

import ue4cv
ue4cv.client.connect()
viewmodes = ['lit', 'depth', 'normal', 'object_mask']

for viewmode in viewmodes:
    print 'Switch to viewmode %s' % viewmode
    res = ue4cv.client.request('vset /viewmode %s' % viewmode)
    assert(res == 'ok')
    time.sleep(1)

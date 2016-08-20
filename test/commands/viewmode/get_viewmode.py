import time, sys, shutil, os
sys.path.append('../../..')
# Use timer to check time.

import ue4cv
ue4cv.client.connect()
viewmodes = ['lit', 'depth', 'normal', 'object_mask', 'screenshot', 'wireframe', 'default']

for viewmode in viewmodes:
    res = ue4cv.client.request('vget /camera/0/%s' % viewmode)
    if os.path.isfile(res):
        shutil.copyfile(res, '%s.png' % viewmode)
        print '%s is saved to %s.png' % (viewmode, viewmode)
    else:
        print 'Error when generate %s' % viewmode
        print res

res = ue4cv.client.request('vget /camera/0/depth depth.exr')
shutil.copyfile(res, 'depth.exr')

time.sleep(5)

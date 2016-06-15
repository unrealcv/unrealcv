import ue4cv

ue4cv.client.connect()
if not ue4cv.client.isconnected():
    print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
else:
    for mode in ['lit', 'depth', 'normal', 'object_mask']:
        filename = ue4cv.client.request('vget /camera/0/%s' % mode)
        print '%s image is saved to %s' % (mode, filename)
    ue4cv.client.disconnect()


from unrealcv import client
client.connect() # Connect to the game
if not client.isconnected(): # Check if the connection is successfully established
    print('UnrealCV server is not running. Run the game from http://unrealcv.github.io first.')
else:
    filename = client.request('vget /camera/0/lit')
    print('Image is saved to %s' % filename)
    for gt_type in ['normal', 'object_mask']:
        filename = client.request('vget /camera/0/%s' % gt_type)
        print('%s is saved to %s' % (gt_type, filename))
    filename = client.request('vget /camera/0/depth depth.exr')
    print('depth is saved to %s' % filename)
    # Depth needs to be saved to HDR image to ensure numerical accuracy

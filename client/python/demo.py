from unrealcv import Client
client = Client(('10.161.159.75', 9000))
# Try to connect our python client to the game
client.connect()

# Check if the connection is successfully established
if not client.isconnected():
    print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
else:
    filename = client.request('vget /camera/0/lit')
    print 'Image is saved to %s' % filename

    filename = client.request('vget /camera/0/depth')
    print 'Depth is saved to %s' % filename

    filename = client.request('vget /camera/0/object_mask')
    print 'Object instance mask is saved to %s' % filename

    filename = client.request('vget /camera/0/normal')
    print 'Surface normal is saved to %s' % filename

    # Disconnect python client from the game
    client.disconnect()

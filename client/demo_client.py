import ue4cv

def subplot_image(index, image):
    plt.subplot(index)
    im = plt.imread(image)
    plt.imshow(im)
    plt.axis('off')

ue4cv.client.connect()
if not ue4cv.client.isconnected():
    print 'UnrealCV server is not running. Run the game downloaded from http://unrealcv.github.io first.'
else:
    filename = ue4cv.client.request('vget /camera/0/lit')
    object_mask_filename = ue4cv.client.request('vget /camera/0/object_mask')
    print 'Image saved to %s' % im
    ue4cv.client.disconnect()

    try:
        import matplotlib.pyplot as plt
        subplot_image(121, filename)
        subplot_image(122, object_mask_filename)
        plt.show()

    except Exception as e:
        print 'matplotlib not installed'
        print e

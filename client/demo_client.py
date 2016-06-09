import ue4cv

def subplot_image(index, image):
    plt.subplot(index)
    im = plt.imread(image)
    plt.imshow(im)
    plt.axis('off')



ue4cv.client.connect()
if not ue4cv.client.isconnected():
    print 'The game is not running, run it first'
else:
    im = ue4cv.client.request('vget /camera/0/lit')
    object_mask = ue4cv.client.request('vget /camera/0/object_mask')
    print 'Image saved to %s' % im
    ue4cv.client.disconnect()

    try:
        import matplotlib.pyplot as plt
        subplot_image(121, im)
        subplot_image(122, object_mask)
        plt.show()

    except Exception as e:
        print 'matplotlib not installed'
        print e

''' Generate image and ground truth from the scene, for development purpose '''

from unrealcv import client
import numpy as np
import matplotlib.pyplot as plt

def read_png(res):
    import StringIO, PIL.Image
    img = PIL.Image.open(StringIO.StringIO(res))
    return np.asarray(img)

def read_npy(res):
    import StringIO
    return np.load(StringIO.StringIO(res))

def request(cmd):
    res = client.request(cmd)
    if res.startswith('error'):
        print(res)
    return res

if __name__ == '__main__':
    client.connect()
    print('Client is connected')

    # Save lit png
    res = request('vget /camera/0/lit png')
    lit_png = read_png(res)
    print(lit_png.shape)
    plt.imsave('lit.png', lit_png)

    res = request('vget /camera/0/normal png')
    normal_png = read_png(res)
    print(normal_png.shape)
    plt.imsave('normal.png', normal_png)

    res = request('vget /camera/0/depth npy')
    depth_npy = read_npy(res)
    print(depth_npy.shape)
    np.save('depth.npy', depth_npy)

    res = request('vget /camera/0/normal npy')
    normal_npy = read_npy(res)
    print(normal_npy.shape)
    np.save('normal.npy', normal_npy)

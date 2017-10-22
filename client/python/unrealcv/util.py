import numpy as np
import PIL.Image
from io import BytesIO
# StringIO module is removed in python3, use io module

def read_png(res):
    '''
    Return a numpy array from binary bytes of png format

    Parameters
    ----------
    res : bytes
        For example, res = client.request('vget /camera/0/lit png')

    Returns
    -------
    numpy.array
        Numpy array
    '''
    img = None
    try:
        PIL_img = PIL.Image.open(BytesIO(res))
        img = np.asarray(PIL_img)
    except:
        print('Read png can not parse response %s' % str(res[:20]))
    return img

def read_npy(res):
    '''
    Return a numpy array from binary bytes of numpy binary file format

    Parameters
    ----------
    res : bytes
        For example, res = client.request('vget /camera/0/depth npy')

    Returns
    -------
    numpy.array
        Numpy array
    '''
    # res is a binary buffer
    arr = None
    try:
        arr = np.load(BytesIO(res))
    except:
        print('Read npy can not parse response %s' % str(res[:20]))
    return arr

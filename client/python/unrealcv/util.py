import numpy as np
import PIL.Image
from io import BytesIO
# StringIO module is removed in python3, use io module

def read_png(res):
    img = None
    try:
        PIL_img = PIL.Image.open(BytesIO(res))
        img = np.asarray(PIL_img)
    except:
        print('Read png can not parse response %s' % str(res[:20]))
    return img

def read_npy(res):
    # res is a binary buffer
    arr = None
    try:
        arr = np.load(BytesIO(res))
    except:
        print('Read npy can not parse response %s' % str(res[:20]))
    return arr

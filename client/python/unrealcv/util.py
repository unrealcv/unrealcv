import numpy as np
import PIL
from io import BytesIO
# StringIO module is removed in python3, use io module

def read_png(res):
    import PIL.Image
    img = PIL.Image.open(BytesIO(res))
    return np.asarray(img)

def read_npy(res):
    # res is a binary buffer
    return np.load(BytesIO(res))

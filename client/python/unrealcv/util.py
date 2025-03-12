import numpy as np
import PIL.Image
from io import BytesIO
import os
import time
# StringIO module is removed in python3, use io module

class ResChecker:
    # Define some utility functions to check whether the response is as expected
    def is_error(self, res):
        return (res is None) or res.startswith('error')

    def is_ok(self, res):
        return res == 'ok'

    def not_error(self, res):
        return not self.is_error(res)

    def is_expected_file_extension(self, path, valid_ext):
        ext = os.path.splitext(path)[-1]
        if ext not in valid_ext:
            print(print(f'Invalid file extension {ext}, should be in {valid_ext}'))
        return ext in valid_ext

def measure_fps(func, *args, **kwargs):
    """
    Measure the frames per second (FPS) of a function.

    Args:
        func (function): The function to measure.
        *args: Variable length argument list for the function.
        **kwargs: Arbitrary keyword arguments for the function.

    Returns:
        float: The measured FPS.
    """
    start_time = time.time()
    for _ in range(60):
        func(*args, **kwargs)
    end_time = time.time()
    elapsed_time = end_time - start_time
    fps = 60 / elapsed_time
    return fps

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


def convert2planedepth(PointDepth, f=320): # convert point depth to plane depth
    """
    Convert point depth to plane depth.

    Args:
        PointDepth (numpy.array): The point depth array.
        f (int): half of the width of the image plane, default is 320.

    Returns:
        numpy.array: The plane depth array.
    """
    H = PointDepth.shape[0]
    W = PointDepth.shape[1]
    i_c = np.float(H) / 2 - 1
    j_c = np.float(W) / 2 - 1
    columns, rows = np.meshgrid(np.linspace(0, W - 1, num=W), np.linspace(0, H - 1, num=H))
    DistanceFromCenter = ((rows - i_c) ** 2 + (columns - j_c) ** 2) ** 0.5
    PlaneDepth = PointDepth / (1 + (DistanceFromCenter / f) ** 2) ** 0.5
    return PlaneDepth

def time_it(func):
    """
    Decorator to measure the execution time of a function.

    Args:
        func (function): The function to measure.

    Returns:
        function: The wrapped function with execution time measurement.
    """
    def wrapper(*args, **kwargs):
        start_time = time.perf_counter()
        result = func(*args, **kwargs)
        end_time = time.perf_counter()
        execution_time = end_time - start_time
        print(f"{func.__name__} executed in {execution_time:.4f} seconds")
        return result
    return wrapper

def parse_resolution(res):
    """
    Parse the resolution string into a tuple of integers.

    Args:
        res (str): The resolution string in the format 'WIDTHxHEIGHT'.

    Returns:
        tuple: The parsed resolution as a tuple (width, height).

    Raises:
        ValueError: If the resolution string is not in the correct format or contains non-integer values.
    """
    resolution = res.split('x')
    if len(resolution) != 2:
        parser.error('Resolution must be specified as WIDTHxHEIGHT')
    try:
        return (int(resolution[0]), int(resolution[1]))
    except ValueError:
        parser.error('WIDTH and HEIGHT must be integers')
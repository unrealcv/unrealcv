# Make sure the configuration file of UnrealCV is working properly
# Need to manually modify the config file to pass these tests
# Written with py.test


# List available configurations
port = 9010
width = 1024
height = 768

from unrealcv import Client
import matplotlib.pyplot as plt
import sys; sys.path.append('..')
import testutil

client = Client(('localhost', port))
def test_port():
    client.connect()
    assert client.isconnected(), 'Can not connect to port %d' % port

def test_res():
    ''' Test whether the resolution configuration is right '''
    client.connect()
    filename = client.request('vget /camera/0/lit')
    filename = testutil.convert_abspath(filename)
    im = plt.imread(filename)
    print im.shape
    assert im.shape == (height, width, 4), 'The image size does not match'

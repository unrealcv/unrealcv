from unrealcv import client
import re

class Color(object):
    ''' A utility class to parse color value '''
    regexp = re.compile('\(R=(.*),G=(.*),B=(.*),A=(.*)\)')
    def __init__(self, color_str):
        self.color_str = color_str
        match = self.regexp.match(color_str)
        (self.R, self.G, self.B, self.A) = [int(match.group(i)) for i in range(1,5)]

    def __repr__(self):
        return self.color_str

    def __eq__(self, o):
        equal = (self.R == other.R and self.G == other.G and self.B == other.B)
        return equal

def get_color_mapping(client, object_list):
    ''' Get the color mapping for specified objects '''
    color_mapping = {}
    for objname in object_list:
        color_mapping[objname] = Color(client.request('vget /object/%s/color' % objname))
    return color_mapping

def test_color_mapping():
    ''' Make sure the color for each object is identical '''
    client.connect()
    scene_objects = client.request('vget /objects').split(' ')
    color_mapping = get_color_mapping(client, scene_objects)
    colors = color_mapping.values()
    assert len(colors) == len(set(colors))
    print 'Number of objects %d, number of unique colors %d' % (len(colors), len(set(colors)))
    print color_mapping

    # Make sure the colors are identical

if __name__ == '__main__':
    test_color_mapping()

import os, sys, time, re
import numpy as np
import matplotlib.pyplot as plt

sys.path.append('..')
import ue4cv


class Timer(object):
    def __init__(self, name=None):
        self.name = name

    def __enter__(self):
        self.tstart = time.time()

    def __exit__(self, type, value, traceback):
        if self.name:
            print '[%s]' % self.name,
        print 'Elapsed: %s' % (time.time() - self.tstart)

class Namedict(object):
    def __init__(self, **kwargs):
        self.dic = kwargs
        self.__dict__.update(self.dic)

    def __repr__(self):
        return self.dic

    def __str__(self):
        return str(self.dic)
# d = namedict(a=1, b=2)
# d.a

def read_camera_info(filename):
    with open(filename) as f:
        lines = f.readlines()
    # Parse camera location and rotation from file
    camera_pos = []
    for line_id in range(len(lines)):
        line = lines[line_id].strip() # Remove \n at the end
        if line_id % 3 == 0: # filename
            pass
        elif line_id % 3 == 1: # location
            location = [float(v) for v in line.split(' ')]
        elif line_id % 3 == 2: # Rotation
            rotation = [float(v) for v in line.split(' ')]
            camera_pos.append((location, rotation))
    return camera_pos

def render_frame(client, pos):
    loc = pos[0] # location
    rot = pos[1] # rotation
    cmd = 'vset /camera/0/location %.3f %.3f %.3f' % (loc[0], loc[1], loc[2])
    response = client.request(cmd)
    assert response == 'ok'
    cmd = 'vset /camera/0/rotation %.3f %.3f %.3f' % (rot[0], rot[1], rot[2])
    response = client.request(cmd)
    assert response == 'ok'
    f = Namedict(
        lit = client.request('vget /camera/0/lit'),
        depth = client.request('vget /camera/0/depth'),
        object_mask = client.request('vget /camera/0/object_mask'),
        normal = client.request('vget /camera/0/normal'),
    )
    return f

# Get the color of each object
class Color(object):
    regexp = re.compile('\(R=(.*),G=(.*),B=(.*),A=(.*)\)')
    def __init__(self, color_str):
        self.color_str = color_str
        match = self.regexp.match(color_str)
        (self.R, self.G, self.B, self.A) = [int(match.group(i)) for i in range(1,5)]

    def __repr__(self):
        return self.color_str

def plot_rendered_frame(f):

    subplot_image(221, f.lit)
    subplot_image(222, f.depth)
    subplot_image(223, f.object_mask)
    subplot_image(224, f.normal)
    # plt.subplots_adjust(wspace=0, hspace=0)
    # plt.tight_layout()

def plot_color(color, title):
    im = np.zeros((100, 100, 4))
    # red = np.array([1.0, 0, 0, 1.0])
    color_array = np.array([color.R, color.G, color.B, color.A]) / 255.0 # repmat
    im = im + color_array
    print title, color
    plt.imshow(im)
    plt.axis('off')
    plt.title(title)

def subplot_image(sub_index, image):
    if isinstance(image, str):
        image = plt.imread(image)
    plt.subplot(sub_index)
    plt.imshow(image)
    plt.axis('off')

def plot_object_region(image, object_mask):
    if isinstance(image, str):
        image = plt.imread(image)
    subplot_image(121, image)
    obj_image = image.copy()
    obj_image[~object_mask] = 0
    subplot_image(122, obj_image)

def plot_object_color(object_list, color_mapping):
    N = len(object_list)
    object_id = 1
    for object_name in object_list:
        color = color_mapping[object_name]
        plt.subplot(1,N,object_id)
        plot_color(color, object_name)
        object_id += 1

def get_color_mapping(client, object_list=None):
    '''
    Get the color mapping of this scene
    '''
    # Get object list
    if not object_list:
        object_list = client.request('vget /objects').split(' ')
    # Get the color mapping for each object
    color_mapping = {}
    for objname in object_list:
        color_mapping[objname] = Color(client.request('vget /object/%s/color' % objname))
    return color_mapping

class ObjectInstanceMap(object):
    def __init__(self, color_object_mask, color_mapping):
        '''
        color_object_mask, in which each object is labeled with a unique color
        color_mapping, maps from object name to a color
        '''
        self.color_object_mask = color_object_mask
        self.color_mapping = color_mapping
        self.dic_object_instance = {}

        # TODO: Make sure one object is only labeled once
        objects_names = self.color_mapping.keys()
        for object_name in objects_names:
            color = self.color_mapping[object_name]
            region = self.match_color(self.color_object_mask * 255.999, [color.R, color.G, color.B], tolerance=2)
            if region.sum() != 0: # Present in the image
                self.dic_object_instance[object_name] = region

    def print_object_size(self):
        '''
        Print the size of each object
        '''
        for object_name in self.dic_object_instance.keys():
            print object_name, self.dic_object_instance[object_name].sum()

    def check_coverage(self):
        '''
        Check the portion of labeled image
        '''
        marked_region = np.zeros(self.color_object_mask.shape[0:2])
        for object_name in self.dic_object_instance.keys():
            marked_region += self.dic_object_instance[object_name]
        assert(marked_region.max() == 1)
        print marked_region.sum() / (marked_region.shape[0] * marked_region.shape[1])

    @staticmethod
    def match_color(color_image, target_color, tolerance=3): # Tolerance is used to solve numerical issue
        match_region = np.ones(color_image.shape[0:2], dtype=bool)
        for c in range(3): # Iterate over three channels
            min_val = target_color[c]-tolerance; max_val = target_color[c]+tolerance
            channel_region = (color_image[:,:,c] >= min_val) & (color_image[:,:,c] <= max_val)
            # channel_region = color_image[:,:,c] == target_color[c]
            match_region &= channel_region
        return match_region

    def count_object_instance(object_mask_filename):
        object_mask = plt.imread(object_mask_filename)
        plt.rcParams['figure.figsize'] = (8.0, 8.0)
        plt.imshow(object_mask)
        plt.axis('off')

        dict_instance_mask = {}
        dict_instance_color = {}
        object_id = 0

        counted_region = np.zeros(object_mask.shape[0:2], dtype=bool)
        while (~counted_region).sum() != 0:
            object_id += 1
            # Count how many unique objects in this scene?
            remaining_pixels = object_mask[~counted_region]
            instance_color = remaining_pixels[0,:]

            instance_mask = (object_mask == instance_color).all(axis=2)
            counted_region += instance_mask

            object_name = 'object%d' % object_id
            dict_instance_mask[object_name] = instance_mask
            dict_instance_color[object_name] = instance_color * 255
        return [dict_instance_mask, dict_instance_color]

if __name__ == '__main__':
    ue4cv.client.connect()

    camera_pos = read_camera_info('./realistic_rendering_camera_info.txt')
    pos = camera_pos[1]
    f = render_frame(ue4cv.client, pos)
    color_object_mask = plt.imread(f.object_mask)

    objects = ['WallPiece1_22', 'SM_Shelving_6', 'SM_Couch_1seat_5', 'SM_Frame_39', 'SM_Shelving_7', 'SM_Shelving_8']
    with Timer():
        color_mapping = get_color_mapping(ue4cv.client, objects)
        # color_mapping = get_selected_color_mapping(ue4cv.client)
    object_instance_map = ObjectInstanceMap(color_object_mask, color_mapping)
    object_instance_map.check_coverage()

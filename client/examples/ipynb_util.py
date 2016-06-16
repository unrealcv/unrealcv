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
        return str(self.dic)

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

def render_frame(client, cam_pose=None):
    ''' If you want to render a frame using current camera position, leave the pos argument to None '''
    if pos is not None:
        # Set camera position
        loc = cam_pose[0] # location
        rot = cam_pose[1] # rotation
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

class Color(object):
    regexp = re.compile('\(R=(.*),G=(.*),B=(.*),A=(.*)\)')
    def __init__(self, color_str):
        self.color_str = color_str
        match = self.regexp.match(color_str)
        (self.R, self.G, self.B, self.A) = [int(match.group(i)) for i in range(1,5)]

    def __repr__(self):
        return self.color_str

def subplot_color(index, color, title):
    plt.subplot(index)
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
        image = imread(image)
    plt.subplot(sub_index)
    plt.imshow(image)
    plt.axis('off')

def plot_image_with_mask(image, mask):
    '''
    Mask is a binary matrix, only mask==1 will be plotted
    '''
    if isinstance(image, str):
        image = imread(image)
    subplot_image(121, image)
    masked_image = image.copy()
    masked_image[~mask] = 0
    subplot_image(122, masked_image)

def plot_object_color(object_list, color_mapping):
    N = len(object_list)
    object_id = 1
    for object_name in object_list:
        color = color_mapping[object_name]
        plt.subplot(1,N,object_id)
        plot_color(color, object_name)
        object_id += 1

def get_color_mapping(client, object_list):
    ''' Get the color mapping for specified objects '''
    color_mapping = {}
    for objname in object_list:
        color_mapping[objname] = Color(client.request('vget /object/%s/color' % objname))
    return color_mapping

class ObjectInstanceMap(object):
    def print_object_size(self):
        '''
        Print the size of each object
        '''
        for object_name in self.dic_object_mask.keys():
            print object_name, self.dic_object_mask[object_name].sum()

    def count_object_instance(object_mask_filename):
        object_mask = imread(object_mask_filename)
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

def generate_objtype(scene_objects):
    # generate_objtype(scene_objects)
    # Use http://www.jsoneditoronline.org/ to clean the json
    # http://jsonformat.com/#jsondataurllabel
    ''' Get object category from object name, with some manual editing '''
    print '{'
    for obj in scene_objects:
        objtype = obj.replace('SM_', '').split('_')[0].replace('BookLP', 'Book').replace('Wire1', 'Wire')
        print ' ', repr(obj), ':', repr(objtype), ','
    print '}'

def match_color(color_image, target_color, tolerance=3): # Tolerance is used to solve numerical issue
    match_region = np.ones(color_image.shape[0:2], dtype=bool)
    for c in range(3): # Iterate over three channels
        min_val = target_color[c]-tolerance; max_val = target_color[c]+tolerance
        channel_region = (color_image[:,:,c] >= min_val) & (color_image[:,:,c] <= max_val)
        # channel_region = color_image[:,:,c] == target_color[c]
        match_region &= channel_region
    return match_region

def compute_instance_mask(object_mask, color_mapping, objects):
    if isinstance(object_mask, str):
        object_mask = misc.imread(object_mask)

    dic_instance_mask = {}
    for object_name in objects:
        color = color_mapping[object_name]
        region = match_color(object_mask, [color.R, color.G, color.B], tolerance=3)
        if region.sum() != 0: # Present in the image
            dic_instance_mask[object_name] = region
    return dic_instance_mask


def check_coverage(dic_instance_mask):
    '''
    Check the portion of labeled image
    '''
    marked_region = None
    for object_name in dic_instance_mask.keys():
        instance_mask = dic_instance_mask[object_name]
        if marked_region is None:
            marked_region = np.zeros(instance_mask.shape[0:2])
        marked_region += instance_mask

    assert(marked_region.max() == 1)
    if marked_region.max() > 1:
        print 'There are invalid regions in the labeling'
    coverage = float(marked_region.sum()) / (marked_region.shape[0] * marked_region.shape[1])
    print 'Coverage %.2f' % coverage
    return marked_region


def get_category_mask(category):
    mask = None
    for obj in image_objects:
        if dic_objects_category[obj] == category:
            if mask is None:
                mask = dic_instance_mask[obj]
            else:
                mask += dic_instance_mask[obj]
    if mask is None:
        print 'Error: the mask is empty'
    return mask

if __name__ == '__main__':
    ue4cv.client.connect()

    camera_pos = read_camera_info('./realistic_rendering_camera_info.txt')
    pos = camera_pos[1]
    f = render_frame(ue4cv.client, pos)
    color_object_mask = imread(f.object_mask)

    objects = ['WallPiece1_22', 'SM_Shelving_6', 'SM_Couch_1seat_5', 'SM_Frame_39', 'SM_Shelving_7', 'SM_Shelving_8']
    with Timer():
        color_mapping = get_color_mapping(ue4cv.client, objects)
        # color_mapping = get_selected_color_mapping(ue4cv.client)
    object_instance_map = ObjectInstanceMap(color_object_mask, color_mapping)
    object_instance_map.check_coverage()

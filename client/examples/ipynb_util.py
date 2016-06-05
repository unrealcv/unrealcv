import matplotlib.pyplot as plt
import numpy as np
import re, time

class Timer(object):
    def __init__(self, name=None):
        self.name = name

    def __enter__(self):
        self.tstart = time.time()

    def __exit__(self, type, value, traceback):
        if self.name:
            print '[%s]' % self.name,
        print 'Elapsed: %s' % (time.time() - self.tstart)

class Namedict:
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)
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
    files = []
    f = Namedict(
        lit = client.request('vget /camera/0/lit'),
        depth = client.request('vget /camera/0/depth'),
        object_mask = client.request('vget /camera/0/object_mask'),
        normal = client.request('vget /camera/0/normal'),
    )
    return f

def plot_rendered_frame(f):
    import matplotlib.gridspec as gridspec
    gs1 = gridspec.GridSpec(2, 2)
    gs1.update(wspace=0, hspace=0)
    plt.rcParams['figure.figsize'] = (18.0, 10.0) # (w, h)

    def subplot(filename, index):
        # plt.subplot(2,2,index)
        plt.subplot(gs1[index-1])
        im = plt.imread(filename)
        plt.imshow(im)
        plt.axis('off')

    subplot(f.lit, 1)
    subplot(f.depth, 2)
    subplot(f.object_mask, 3)
    subplot(f.normal, 4)
    # plt.subplots_adjust(wspace=0, hspace=0)
    # plt.tight_layout()

# Get the color of each object
class Color:
    regexp = re.compile('\(R=(.*),G=(.*),B=(.*),A=(.*)\)')
    def __init__(self, color_str):
        self.color_str = color_str
        match = self.regexp.match(color_str)
        (self.R, self.G, self.B, self.A) = [int(match.group(i)) for i in range(1,5)]

    def __repr__(self):
        return self.color_str


def plot_color(color, title):
    im = np.zeros((100, 100, 4))
    red = np.array([1.0, 0, 0, 1.0])
    color_array = np.array([color.R, color.G, color.B, color.A]) / 255.0 # repmat
    im = im + color_array
    print title, color
    plt.imshow(im)
    plt.axis('off')
    plt.title(title)

def get_color_for_selected_objects(client):
    color_mapping = {}
    objects = ['WallPiece124', 'SM_Shelving8', 'SM_Couch_1seat7', 'SM_Frame41']
    for objname in objects:
        color_mapping[objname] = Color(client.request('vget /object/%s/color' % objname))

    wall_color = color_mapping['WallPiece124']
    shelf_color = color_mapping['SM_Shelving8']
    coach_color = color_mapping['SM_Couch_1seat7']
    frame_color = color_mapping['SM_Frame41']

    plt.subplot(1,4,1)
    plot_color(wall_color, 'wall')
    plt.subplot(1,4,2)
    plot_color(shelf_color, 'shelf')
    plt.subplot(1,4,3)
    plot_color(coach_color, 'coach')
    plt.subplot(1,4,4)
    plot_color(frame_color, 'frame')
    return color_mapping

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
    id = 0

    counted_region = np.zeros(object_mask.shape[0:2], dtype=bool)
    while (~counted_region).sum() != 0:
        id += 1
        # Count how many unique objects in this scene?
        remaining_pixels = object_mask[~counted_region]
        instance_color = remaining_pixels[0,:]

        instance_mask = (object_mask == instance_color).all(axis=2)
        counted_region += instance_mask

        object_name = 'object%d' % id
        dict_instance_mask[object_name] = instance_mask
        dict_instance_color[object_name] = instance_color * 255
    return [dict_instance_mask, dict_instance_color]

def check_coverage(dic_object_instance):
    '''
    Check the portion of labeled image
    '''
    pass

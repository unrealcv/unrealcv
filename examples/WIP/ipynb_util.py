import os, sys, time, re
import numpy as np
import matplotlib.pyplot as plt

class Timer(object):
    def __init__(self, name=None):
        self.name = name

    def __enter__(self):
        self.tstart = time.time()

    def __exit__(self, type, value, traceback):
        if self.name:
            print '[%s]' % self.name,
        print 'Elapsed: %s' % (time.time() - self.tstart)

def plot_object_color(object_list, color_mapping):
    N = len(object_list)
    object_id = 1
    for object_name in object_list:
        color = color_mapping[object_name]
        plt.subplot(1,N,object_id)
        plot_color(color, object_name)
        object_id += 1

def generate_objectcatetory_json(scene_objects):
    # Use http://www.jsoneditoronline.org/ to clean the json
    # http://jsonformat.com/#jsondataurllabel
    ''' Get object category from object name, with some manual editing '''
    print '{'
    for obj in scene_objects:
        objtype = obj.replace('SM_', '').split('_')[0].replace('BookLP', 'Book').replace('Wire1', 'Wire')
        print ' ', repr(obj), ':', repr(objtype), ','
    print '}'

def check_coverage(dic_instance_mask):
    ''' Check the portion of labeled image '''
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

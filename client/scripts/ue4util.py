import os, sys

def get_project_name(projectfile):
    if not projectfile.endswith('.uproject'):
        print 'Error %s is not a valid project file' % projectfile
    return os.path.basename(projectfile).replace('.uproject', '')

def get_platform_name():
    platform = sys.platform # Map from python platform name to ue4 platform name
    names = {
        'cygwin': 'Win64', # could be win32 also
        'win32': 'Win64',
        'win64': 'Win64',
        'linux2': 'Linux',
        'darwin': 'Mac',
        }
    return names[platform]

def platform_function(**kwargs):
    platform_name = get_platform_name()
    if kwargs.get(platform_name):
        return kwargs[platform_name]
    else:
        raise Exception('Function not defined for platform %s' % platform_name)
        return None

def cygwinpath_to_winpath(path):
    path_mapping = {
        # Support cygwin
        "/drives/d/": "D:/",
        "/drives/c/": "C:/",
        "/cygdrive/d/": "D:/",
        "/cygdrive/c/": "C:/",
        "/home/mobaxterm/d/": "D:/",
    }
    for (k, v) in path_mapping.iteritems():
        path = path.replace(k, v)
    # return path.replace('/drives/d/', 'D:/').replace('/drives/c/', 'C:/')
    return path


def get_real_abspath(filename):
    '''
    Get the correct abs path is tricky, this function is designed to achive that
    '''
    # Convert path in python to real path on the disk, useful for windows
    if len(filename) >= 3 and filename[1] == ':':
        pass # skip windows absolute path
    else:
        filename = os.path.abspath(filename)

    if sys.platform == 'cygwin':
        filename = cygwinpath_to_winpath(filename)
    return filename

def run_ue4cmd(cmd, debug=False):
    print(cmd)
    if not debug:
        os.system(cmd)

def mkdirp(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

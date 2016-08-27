import os, sys

def get_project_name(projectfile):
    if not projectfile.endswith('.uproject'):
        print 'Error %s is not a valid project file' % projectfile
    return os.path.basename(projectfile).replace('.uproject', '')

def get_platform_name():
    platform = sys.platform
    names = {
        'cygwin': 'win',
        'win32': 'win',
        'win64': 'win',
        'linux2': 'linux',
        'darwin': 'mac',
        }
    return names[platform]

def platform_function(**kwargs):
    platform_name = get_platform_name()
    if kwargs.get(platform_name):
        return kwargs[platform_name]
    else:
        raise Exception('Function not defined for platform %s' % platform_name)
        return None

def get_project_infofile(project_name, project_output_folder):
    info_filename = os.path.join(get_output_root_folder(project_output_folder), '%s-info.txt' % project_name)
    return info_filename

def cygwinpath_to_winpath(path):
    path_mapping = {
        "/drives/d/": "D:/",
        "/drives/c/": "C:/",
        "/home/mobaxterm/d/": "D:/",
    }
    for (k, v) in path_mapping.iteritems():
        path = path.replace(k, v)
    # return path.replace('/drives/d/', 'D:/').replace('/drives/c/', 'C:/')
    return path

def get_output_root_folder(project_output_folder):
    ue4buildtype = {
        'mac': 'MacNoEditor',
        'linux': 'LinuxNoEditor',
        'win': 'WindowsNoEditor',
    }
    platform_name = get_platform_name()
    if not ue4buildtype.get(platform_name):
        raise Exception('Platform %s is not supported' % platform_name)
    else:
        buildtype = ue4buildtype[platform_name]
        output_root_folder = os.path.join(project_output_folder, buildtype)

        return output_root_folder

def get_real_abspath(filename):
    # Convert path in python to real path on the disk, useful for windows
    if len(filename) >= 3 and filename[1] == ':':
        pass # skip windows absolute path
    else:
        filename = os.path.abspath(filename)

    if sys.platform == 'cygwin':
        filename = cygwinpath_to_winpath(filename)
    return filename

def run_ue4cmd(cmd):
    print(cmd)
    os.system(cmd)

def mkdirp(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

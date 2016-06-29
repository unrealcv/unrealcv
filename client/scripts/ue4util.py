import os, sys
from ue4config import conf

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

def get_project_infofile(project_name):
    info_filename = os.path.join(get_output_root_folder(), '%s-info.txt' % project_name)
    return info_filename

def cygwinpath_to_winpath(path):
    return path.replace('/drives/d/', 'D:/').replace('/drives/c/', 'C:/')

def get_output_root_folder():
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
        output_root_folder = os.path.join(conf['OutputFolder'], buildtype)

        return output_root_folder

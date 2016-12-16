import os, sys, argparse


EnginePathNotDefined = '''
EnginePath is not specified, script is not sure where to find UnrealEngine
configure it by copy ue4config.py.example to ue4config.py and change the EnginePath
or use argument --engine_path to specify it
'''

EngineNotFound = '''
UE4 can not be found in %s, check ue4config.py and argument to make sure it is correct
'''

UATScriptNotFound = '''
RunUAT script can not be found in %s
'''

class UE4Util:
    def __init__(self, engine_path):
        if not os.path.isdir(engine_path):
            exit(EngineNotFound % engine_path)
        self.engine_path = engine_path

    def get_UAT_script(self):
        '''
        UAT Script is a command line script for automation
        '''
        platform = get_platform_name()

        if platform == 'Win64':
            UAT_basename = 'Engine/Build/BatchFiles/RunUAT.bat'
        elif platform == 'Mac' or platform == 'Linux':
            UAT_basename = 'Engine/Build/BatchFiles/RunUAT.sh'
        else:
            print 'platform %s is not recognized' % platform

        UAT_script = os.path.join(self.engine_path, UAT_basename)
        if not os.path.isfile(UAT_script):
            exit(UATScriptNotFound % UAT_script)

        # return UAT_script.replace(' ', '\ ')
        return UAT_script

def get_project_name(projectfile):
    if not projectfile.endswith('.uproject'):
        print 'Error %s is not a valid project file' % projectfile
    return os.path.basename(projectfile).replace('.uproject', '')


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
    filename = filename.replace('\\', '/') # This is much more universal and easier to handle
    return filename

def run_ue4cmd(cmd, debug=False):
    print(cmd)
    if not debug:
        # os.system(cmd)

        import subprocess
        subprocess.call([cmd], shell=True)

def mkdirp(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

def get_path_sep():
    # if get_platform_name() == 'Win64':
    #     return '\\'
    # else: # Linux, Mac, cygwin
    #     return '/'
    return '/'

import os

def getprojectname(projectfile):
    if not projectfile.endswith('.uproject'):
        print 'Error %s is not a valid project file' % projectfile
    return os.path.basename(projectfile).replace('.uproject', '')

def platform_name():
    platform = sys.platform
    names = {
        'cygwin': 'win',
        'win32': 'win',
        'win64': 'win',
        'linux2': 'linux',
        }
    return names[platform]

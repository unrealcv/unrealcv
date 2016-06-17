import os, sys, argparse
from ue4util import *
from ue4config import conf

# Automatically package game content
def package_linux(conf, projectfile):
    cmd = '{UATScript} BuildCookRun -project={projectfile} -archivedirectory={outputfolder} -noP4 -platform=Linux -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'.format(
        projectfile = projectfile,
        UATScript = conf['UATScript'],
        outputfolder = conf['OutputFolder'],
    )
    os.system(cmd)

def package_mac(conf, projectfile):
    cmd = '{UATScript} BuildCookRun -project={projectfile} -archivedirectory={outputfolder} -noP4 -platform=Mac -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'.format(
        projectfile = projectfile,
        UATScript = conf['UATScript'],
        outputfolder = conf['OutputFolder'],
    )
    print cmd
    os.system(cmd)

def package_win(conf, projectfile):
    cmd = '"{UATScript}" BuildCookRun -project="{projectfile}" -archivedirectory={outputfolder} -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'.format(
        projectfile = projectfile,
        UATScript = conf['UATScript'],
        outputfolder = conf['OutputFolder'],
    )
    # windows_cmd = 'cmd /C start "" "D:/Epic Games/4.11/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project=\"D:/Unreal Projects/FlyingCppDefault/FlyingCppDefault.uproject\" -archivedirectory=AutoPackaging -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'
    # In windows, directly run shell command is a little tricky. Easier to save a bat file first.
    with open('package.bat', 'w') as f:
        f.write(cmd)

    windows_cmd = 'cmd /C start "" "package.bat"'
    os.system(windows_cmd)

platform = sys.platform
if platform == 'Win64' or platform == 'cygwin':
    package = package_win

if platform == 'Linux' or platform == 'linux2':
    package = package_linux

if platform == 'darwin':
    package = package_mac

def save_version_info(conf, project_file):
    ''' Save the version info of UnrealCV plugin and the game for easier issue tracking'''
    project_name = getprojectname(project_file)

    project_folder = os.path.dirname(project_file)
    plugin_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')

    plugin_version = os.popen('git -C %s rev-parse --short HEAD' % plugin_folder).read().strip()
    assert(len(plugin_version) == 7)
    project_version = os.popen('git -C %s rev-parse --short HEAD' % project_folder).read().strip()
    assert(len(project_version) == 7)

    info_filename = os.path.join(conf['OutputFolder'], project_name)
    info = '''
    Project         : {project_name}
    Project Version : {project_version}
    Plugin Version  : {plugin_version}
    '''.format(project_name=project_name, project_version=project_version, plugin_version=plugin_version)
    with open(info_filename, 'w') as info_file:
        info_file.write(info)

    print plugin_version
    print project_version
    # git rev-parse --short HEAD

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    args = parser.parse_args()

    project_file = os.path.abspath(args.project_file).replace('/drives/d/', 'D:/').replace('/home/mobaxterm/d/', 'D:/')
    # package(conf, project_file)
    save_version_info(conf, project_file)

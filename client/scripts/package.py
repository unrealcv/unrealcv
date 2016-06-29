import os, sys, argparse, json
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

package = platform_function(
    win = package_win,
    mac = package_mac,
    linux = package_linux
)

def is_dirty(git_repo):
    status = os.popen('git -C %s status -s' % git_repo).read().strip()
    if status != '':
        print 'Folder %s has uncommited changes' % git_repo
        print status
        return True
    return False


def save_version_info(conf, project_file):
    ''' Save the version info of UnrealCV plugin and the game for easier issue tracking'''
    project_name = get_project_name(project_file)

    project_folder = os.path.dirname(project_file)
    plugin_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')

    if is_dirty(project_folder):
        return False

    if is_dirty(plugin_folder):
        return False

    plugin_version = os.popen('git -C %s rev-parse --short HEAD' % plugin_folder).read().strip()
    assert(len(plugin_version) == 7)
    project_version = os.popen('git -C %s rev-parse --short HEAD' % project_folder).read().strip()
    assert(len(project_version) == 7)

    info = dict(
        project_name = project_name,
        project_version = project_version,
        plugin_version = plugin_version,
        platform = get_platform_name(),
    )
    info_filename = get_project_infofile(project_name)
    with open(info_filename, 'w') as f:
        json.dump(info, f, indent = 4)
    print 'Save project info to file %s' % info_filename
    print json.dumps(info, indent = 4)

    return True
    # git rev-parse --short HEAD

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    args = parser.parse_args()

    project_file = os.path.abspath(args.project_file)
    if sys.platform == 'cygwin':
        project_file = cygwinpath_to_winpath(project_file)

    if save_version_info(conf, project_file):
        package(conf, project_file)
        save_version_info(conf, project_file) # Save the info file again, in case it is deleted by Unreal Engine script

import os, sys, argparse, json
import ue4util
from ue4config import conf
import gitutil

UATScriptTemplate = '{UATScript} BuildCookRun -project={ProjectFile} -archivedirectory={OutputFolder} -noP4 -platform={Platform} -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'

UATParams = dict(
    UATScript = conf['UATScript'],
    OutputFolder = conf['OutputFolder'],
)

def package(conf, projectfile):
    Platform = ue4util.get_platform_name()
    cmd = UATScriptTemplate.format(**UATParams)
    ue4util.run_ue4cmd(cmd)

def save_version_info(conf, project_file):
    ''' Save the version info of UnrealCV plugin and the game for easier issue tracking'''
    project_name = ue4util.get_project_name(project_file)

    project_folder = os.path.dirname(project_file)
    plugin_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')

    if gitutil.is_dirty(project_folder):
        return False

    if gitutil.is_dirty(plugin_folder):
        return False

    plugin_version = gitutil.get_short_version(plugin_folder)
    assert(len(plugin_version) == 7)
    project_version = gitutil.get_short_version(project_folder)
    assert(len(project_version) == 7)

    info = dict(
        project_name = project_name,
        project_version = project_version,
        plugin_version = plugin_version,
        platform = ue4util.get_platform_name(),
    )
    info_filename = ue4util.get_project_infofile(project_name)
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

    project_file = ue4util.get_real_path(args.project_file)

    if save_version_info(conf, project_file): # Check version info, not really doing anything
        package(conf, project_file)
        save_version_info(conf, project_file) # Save the info file again, in case it is deleted by Unreal Engine script

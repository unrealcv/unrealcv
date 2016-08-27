'''
Package *.uproject and release it to output location defined in ue4config.py
'''
import os, sys, argparse, json
import ue4util
from ue4config import conf
import gitutil

def package(project_file, project_output_folder):
    '''
    Build project
    '''

    UATScriptTemplate = '{UATScript} BuildCookRun -project={ProjectFile} -archivedirectory={OutputFolder} -noP4 -platform={Platform} -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'

    cmd = UATScriptTemplate.format(
        UATScript = conf['UATScript'],
        Platform = ue4util.get_platform_name(),
        OutputFolder = 'built',
        ProjectFile = project_file
        )

    ue4util.run_ue4cmd(cmd)

def save_version_info(project_file, project_output_folder):
    ''' Save the version info of UnrealCV plugin and the game for easier issue tracking'''
    project_name = ue4util.get_project_name(project_file)

    project_folder = os.path.dirname(project_file)
    plugin_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')

    if gitutil.is_dirty(project_folder):
        return False

    if gitutil.is_dirty(plugin_folder):
        return False

    plugin_version = gitutil.get_short_version(plugin_folder)
    assert(len(plugin_version) == 7 or len(plugin_version) == 0)
    project_version = gitutil.get_short_version(project_folder)
    assert(len(project_version) == 7 or len(project_version) == 0)

    info = dict(
        project_name = project_name,
        project_version = project_version,
        plugin_version = plugin_version,
        platform = ue4util.get_platform_name(),
    )
    info_filename = ue4util.get_project_infofile(project_name, project_output_folder)
    ue4util.mkdirp(os.path.dirname(info_filename))

    with open(info_filename, 'w') as f:
        json.dump(info, f, indent = 4)
        
    print 'Save project info to file %s' % info_filename
    print json.dumps(info, indent = 4)

    return True

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    args = parser.parse_args()

    project_file = ue4util.get_real_abspath(args.project_file)
    project_output_folder = './built_project/%s' % ue4util.get_project_name(project_file)

    if save_version_info(project_file, project_output_folder):
        # Check version info, not really doing anything
        package(project_file, project_output_folder)
        save_version_info(project_file, project_output_folder) # Save the info file again, in case it is deleted by Unreal Engine script

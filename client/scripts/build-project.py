'''
Package *.uproject and release it to output location defined in ue4config.py
'''
import os, sys, argparse, json, shutil, subprocess
import ue4util, gitutil, ziputil, uploadutil

def package(project_file, project_output_folder):
    '''
    Build project
    '''

    UATScriptTemplate = '{UATScript} BuildCookRun -project={ProjectFile} -archivedirectory={OutputFolder} -noP4 -platform={Platform} -clientconfig=Development -serverconfig=Development -cook -allmaps -stage -pak -archive -build'
    # See help information in Engine/Source/Programs/AutomationTool/AutomationUtils/ProjectParams.cs

    # cmd = UATScriptTemplate.format(
    #     UATScript = ue4util.get_UAT_script(),
    #     Platform = ue4util.get_platform_name(),
    #     OutputFolder = ue4util.get_real_abspath(project_output_folder),
    #     ProjectFile = project_file
    #     )
    cmd = [
        ue4util.get_UAT_script(),
        'BuildCookRun',
        '-project=%s' % project_file,
        '-archivedirectory=%s' % ue4util.get_real_abspath(project_output_folder),
        '-noP4',
        '-platform=%s' % ue4util.get_platform_name(),
        '-clientconfig=Development',
        '-serverconfig=Development',
        '-cook',
        '-allmaps',
        '-stage',
        '-pak',
        '-archive',
        '-build'
    ]

    # ue4util.run_ue4cmd(cmd, False)
    subprocess.call(cmd)

def save_version_info(info_filename, project_file, project_output_folder, plugin_version):
    ''' Save the version info of UnrealCV plugin and the game for easier issue tracking'''
    project_name = ue4util.get_project_name(project_file)
    project_folder = os.path.dirname(project_file)

    if gitutil.is_dirty(project_folder):
        return False

    # if gitutil.is_dirty(plugin_folder):
    #     return False
    # plugin_version = gitutil.get_short_version(plugin_folder)
    assert(len(plugin_version) == 7 or len(plugin_version) == 0)
    project_version = gitutil.get_short_version(project_folder)
    assert(len(project_version) == 7 or len(project_version) == 0)

    info = dict(
        project_name = project_name,
        project_version = project_version,
        plugin_version = plugin_version,
        platform = ue4util.get_platform_name(),
    )

    ue4util.mkdirp(os.path.dirname(info_filename))

    with open(info_filename, 'w') as f:
        json.dump(info, f, indent = 4)

    print 'Save project info to file %s' % info_filename
    # print json.dumps(info, indent = 4)

    return True

def main():
    # Files is relative to this python script
    cur_dir = os.path.dirname(os.path.abspath(__file__))

    # Parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    parser.add_argument('--engine_path') # Add here just as a placeholder
    # args = parser.parse_args()
    args, _ = parser.parse_known_args()

    # Read project information
    project_file = ue4util.get_real_abspath(args.project_file)
    project_name = ue4util.get_project_name(project_file)
    project_output_folder = os.path.join(cur_dir, 'built_project/%s' % project_name)
    info_filename = os.path.join(project_output_folder, '%s-info.txt' % project_name)

    # Get plugin information
    project_folder = os.path.dirname(project_file)
    plugin_infofile = os.path.join(project_folder, 'Plugins', 'unrealcv', 'unrealcv-info.txt')
    if not os.path.isfile(plugin_infofile):
        print 'Plugin not exist'
        return
    else:
        with open(plugin_infofile, 'r') as f:
            plugin_info = json.load(f)
            plugin_version = plugin_info['plugin_version']

    if os.path.isdir(project_output_folder):
        print 'Output directory %s exist, delete it first' % project_output_folder
    else:
        # Package game and save version info
        package(project_file, project_output_folder)

        # Save version info after build finished
        save_version_info(info_filename, project_file, project_output_folder, plugin_version)


if __name__ == '__main__':
    main()

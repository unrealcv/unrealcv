'''
Package *.uproject and release it to output location defined in ue4config.py
'''
import os, sys, argparse, json, shutil
import ue4config
import ue4util, gitutil, ziputil, uploadutil

def package(project_file, project_output_folder):
    '''
    Build project
    '''

    UATScriptTemplate = '{UATScript} BuildCookRun -project={ProjectFile} -archivedirectory={OutputFolder} -noP4 -platform={Platform} -clientconfig=Development -serverconfig=Development -cook -allmaps -stage -pak -archive -clean -build'
    # See help information in Engine/Source/Programs/AutomationTool/AutomationUtils/ProjectParams.cs

    cmd = UATScriptTemplate.format(
        UATScript = ue4config.conf['UATScript'].replace(' ', '\ '),
        Platform = ue4util.get_platform_name(),
        OutputFolder = ue4util.get_real_abspath(project_output_folder),
        ProjectFile = project_file
        )

    ue4util.run_ue4cmd(cmd)

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

def get_files_win(output_folder, project_name):
    info_filename = os.path.join(output_folder, '%s-info.txt' % project_name)
    root = os.path.join(output_folder, 'WindowsNoEditor')
    files = [
        os.path.join(root, '%s.exe' % project_name),
        os.path.join(root, '%s/' % project_name),
        os.path.join(root, 'Engine/'),
        info_filename,
    ]
    return files

def get_files_mac(output_folder, project_name):
    info_filename = os.path.join(output_folder, '%s-info.txt' % project_name)
    root = os.path.join(output_folder, 'MacNoEditor')
    files = [
        os.path.join(root, '%s.app' % project_name),
        info_filename
    ]
    return files

def get_files_linux(output_folder, project_name):
    info_filename = os.path.join(output_folder, '%s-info.txt' % project_name)
    root = os.path.join(output_folder, 'LinuxNoEditor')
    files = [
        os.path.join(root, 'Engine/'),
        os.path.join(root, project_name), #binary
        info_filename
    ]
    return files

get_files = ue4util.platform_function(
    Mac = get_files_mac,
    Win64 = get_files_win,
    Linux = get_files_linux,
)

def get_zipfilename_from_infofile(project_infofile, plugin_infofile):
    with open(project_infofile, 'r') as f:
        project_info = json.load(f)
    with open(plugin_infofile, 'r') as f:
        plugin_info = json.load(f)
    info = project_info.copy()
    info.update(plugin_info)
    return '{project_name}-{platform}-{project_version}-{plugin_version}.zip'.format(**info)

def check_files_ok(files):
    isok = True
    for file in files:
        if os.path.isfile(file) or os.path.isdir(file):
            continue
        else:
            print 'File %s not exist, stop packaging' % file
            isok = False
    return isok

def zip_project(zipfilename, project_file, project_output_folder):
    project_name = ue4util.get_project_name(project_file)
    files = get_files(project_output_folder, project_name)

    zip_root_folder = ue4util.get_real_abspath(project_output_folder) + '/'
    if check_files_ok(files): # Make sure all files we want exist
        all_files = ziputil.get_all_files(files)
        print 'Start zipping files to %s, Zip root folder is %s' % (zipfilename, zip_root_folder)
        ziputil.zipfiles(all_files, zip_root_folder, zipfilename, verbose=False)
        return True
    else:
        return False

def main():
    # Parse command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    args = parser.parse_args()

    # Read project information
    project_file = ue4util.get_real_abspath(args.project_file)
    project_name = ue4util.get_project_name(project_file)
    project_output_folder = './built_project/%s' % project_name
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

    # Package game and save version info
    package(project_file, project_output_folder)

    # Save version info after build finished
    save_version_info(info_filename, project_file, project_output_folder, plugin_version)

    # Zip files
    '''
    zipfilename = os.path.join(project_output_folder, get_zipfilename_from_infofile(info_filename))
    zip_project(zipfilename, project_file, project_output_folder)

    # Upload built games to output targets
    upload_confs = ue4config.conf['ProjectOutput']
    upload_handlers = dict(
        scp = uploadutil.upload_scp,
        s3 = uploadutil.upload_s3,
    )
    for upload_conf in upload_confs:
        tgt_type = upload_conf['Type']
        upload_handlers[tgt_type](upload_conf, [zipfilename], os.path.dirname(zipfilename))
    '''

if __name__ == '__main__':
    main()

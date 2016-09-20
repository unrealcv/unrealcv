import argparse, os, json
import ue4util, ziputil, uploadutil

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

def check_files_ok(files):
    isok = True
    for file in files:
        if os.path.isfile(file) or os.path.isdir(file):
            continue
        else:
            print 'File %s not exist, stop packaging' % file
            isok = False
    return isok

def zip_project(zipfilename, project_name, project_output_folder):
    files = get_files(project_output_folder, project_name)

    zip_root_folder = ue4util.get_real_abspath(project_output_folder) + '/'
    if check_files_ok(files): # Make sure all files we want exist
        all_files = ziputil.get_all_files(files)
        print 'Start zipping files to %s, Zip root folder is %s' % (zipfilename, zip_root_folder)
        ziputil.zipfiles(all_files, zip_root_folder, zipfilename, verbose=False)
        return True
    else:
        return False

# def get_zipfilename_from_infofile(project_infofile, plugin_infofile):
def get_zipfilename_from_infofile(project_infofile):
    with open(project_infofile, 'r') as f:
        project_info = json.load(f)
    # with open(plugin_infofile, 'r') as f:
    #     plugin_info = json.load(f)
    # info = project_info.copy()
    # info.update(plugin_info)
    info = project_info
    return '{project_name}-{platform}-{project_version}-{plugin_version}.zip'.format(**info)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('project', help='Project name')
    parser.add_argument('remote')
    args = parser.parse_args()

    remote = args.remote
    project_name = args.project

    cur_dir = os.path.dirname(os.path.abspath(__file__))
    project_output_folder = os.path.join(cur_dir, 'built_project/%s' % project_name)
    info_filename = os.path.join(project_output_folder, '%s-info.txt' % project_name)

    # project_folder = os.path.dirname(project_file)
    # plugin_infofile = os.path.join(project_folder, 'Plugins', 'unrealcv', 'unrealcv-info.txt')

    # Zip files
    zipfilename = os.path.join(project_output_folder, get_zipfilename_from_infofile(info_filename))
    print 'Zip project to file %s' % zipfilename
    zip_project(zipfilename, project_name, project_output_folder)

    # Upload built games to output targets
    print 'Upload project from %s -> %s' % (zipfilename, remote)
    uploadutil.upload_scp(remote, [zipfilename], os.path.dirname(zipfilename), DEBUG=True)

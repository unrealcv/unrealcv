import math, sys, argparse, zipfile, json
from ue4config import conf
from ue4util import *
from pyupload import upload_s3, upload_scp

# Even empty folder needs to be preserved
def get_all_files(files):
    all_files = []
    for path in files:
        # if os.path.isfile(path):
        all_files.append(path)

        if os.path.isdir(path):
            for dirname, subdirs, files in os.walk(path):
                for filename in files:
                    all_files.append(os.path.join(dirname, filename))

                for subdir in subdirs:
                    all_files.append(os.path.join(dirname, subdir))

    return [os.path.abspath(v) for v in all_files]



def zipfiles(srcfiles, srcroot, dst):
    print 'zip file'
    zf = zipfile.ZipFile("%s" % (dst), "w", zipfile.ZIP_DEFLATED)

    for srcfile in srcfiles:
        arcname = srcfile[len(srcroot) + 1:]
        print 'zipping %s as %s' % (srcfile,
                                    arcname)
        zf.write(srcfile, arcname)

    zf.close()



def get_files_win(project_name):
    output_folder = get_output_root_folder()
    files = [
        os.path.join(output_folder, '%s.exe' % project_name),
        os.path.join(output_folder, '%s/' % project_name),
        os.path.join(output_folder, 'Engine/'),
        get_project_infofile(project_name),
    ]

    files = [v.replace('D:/', '/drives/d/') for v in files]
    # output_folder = output_folder.replace('D:/', '/drives/d/')

    return files

def get_files_mac(project_name):
    output_folder = get_output_root_folder()
    files = [
        os.path.join(output_folder, '%s.app' % project_name),
        get_project_infofile(project_name),
    ]

    return files

def get_files_linux(project_name):
    output_folder = get_output_root_folder()
    files = [
        os.path.join(output_folder, 'Engine/'),
        os.path.join(output_folder, project_name),
        get_project_infofile(project_name),
    ]
    return files

def check_files_ok(files):
    isok = True
    for file in files:
        if os.path.isfile(file) or os.path.isdir(file):
            continue
        else:
            print 'File %s not exist, stop packaging' % file
            isok = False
    return isok

def get_zipfilename_from_infofile(infofile):
    with open(infofile, 'r') as f:
        info = json.load(f)
        print info
    return '{project_name}-{platform}-{project_version}-{plugin_version}.zip'.format(**info)


get_files = platform_function(
    mac = get_files_mac,
    win = get_files_win,
    linux = get_files_linux,
)

if __name__ == '__main__':
    # From argparse
    # uproject = '/drives/d/UnrealProjects/VehicleAdvanced/VehicleAdvanced.uproject'
    parser = argparse.ArgumentParser()
    parser.add_argument('project_file')
    args = parser.parse_args()
    project_file = args.project_file

    project_name = get_project_name(project_file)
    print 'Files to upload'
    files = get_files(project_name)
    for f in files:
        print f

    if check_files_ok(files):
        platform_name = get_platform_name()
        print 'Project info'
        info_filename = get_project_infofile(project_name)

        zipfilename = get_zipfilename_from_infofile(info_filename)
        zip_root_folder = get_output_root_folder()

        print 'Expanded file list'
        all_files = get_all_files(files)

        if sys.platform == 'cygwin':
            all_files = [cygwinpath_to_winpath(f) for f in all_files]

        for f in all_files:
            print f

        print 'Start zipping files to %s, Zip root folder is %s' % (zipfilename, zip_root_folder)
        zipfiles(all_files, zip_root_folder, zipfilename)

        '''
        print 'Start uploading file %s' % zipfilename
        bucket_name = 'unrealcv-scene'
        upload_s3(bucket_name, zipfilename)
        '''
        upload_scp(zipfilename)

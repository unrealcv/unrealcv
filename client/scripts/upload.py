import math, sys, argparse, zipfile, json
import boto
from filechunkio import FileChunkIO
from ue4config import conf
from ue4util import *


# bucket_name = 'unrealcv-scene'
bucket_name = 'unreal-scene'

def zipfiles(srcfiles, srcroot, dst):
    # zf = zipfile.ZipFile("%s.zip" % (dst), "w", zipfile.ZIP_DEFLATED)
    print 'zip file'
    zf = zipfile.ZipFile("%s" % (dst), "w", zipfile.ZIP_DEFLATED)
    # abs_src = os.path.abspath(src)
    # abs_src_folder = os.path.dirname(abs_src)
    srcroot = os.path.abspath(srcroot)
    for srcfile in srcfiles:
        srcfile = os.path.abspath(srcfile)

        if os.path.isfile(srcfile):
            arcname = srcfile[len(srcroot) + 1:]
            print 'zipping %s as %s' % (srcfile,
                                        arcname)
            zf.write(srcfile, arcname)

        elif os.path.isdir(srcfile):
            for dirname, subdirs, files in os.walk(srcfile):
                for filename in files:
                    absname = os.path.abspath(os.path.join(dirname, filename))
                    # arcname = absname[len(abs_src) + 1:]
                    arcname = absname[len(srcroot) + 1:]
                    print 'zipping %s as %s' % (os.path.join(dirname, filename),
                                                arcname)
                    zf.write(absname, arcname)

        else:
            print 'File %s is not found' % srcfile
    zf.close()

def upload(bucket_name, filename):
    # From http://boto.cloudhackers.com/en/latest/s3_tut.html
    # from boto.s3.connection import S3Connection
    S3_ACCESS_KEY = os.environ['AWS_ACCESS_KEY_ID']
    S3_SECRET_KEY = os.environ['AWS_SECRET_ACCESS_KEY']
    # conn = S3Connection(S3_ACCESS_KEY, S3_SECRET_KEY)
    print S3_ACCESS_KEY
    print S3_SECRET_KEY

    # Connect to S3
    # c = boto.connect_s3(host='s3-ap-northeast-1.amazonaws.com')
    c = boto.connect_s3(host='s3-us-west-1.amazonaws.com')
    b = c.get_bucket(bucket_name)

    # Get file info
    source_path = filename
    source_size = os.stat(source_path).st_size

    # Create a multipart upload request
    mp = b.initiate_multipart_upload(os.path.basename(source_path))

    # Use a chunk size of 50 MiB (feel free to change this)
    chunk_size = 52428800
    chunk_count = int(math.ceil(source_size / float(chunk_size)))

    # Send the file parts, using FileChunkIO to create a file-like object
    # that points to a certain byte range within the original file. We
    # set bytes to never exceed the original file size.
    for i in range(chunk_count):
        print 'Uploading chunk %d/%d' % (i, chunk_count)
        offset = chunk_size * i
        bytes_to_send = min(chunk_size, source_size - offset)
        with FileChunkIO(source_path, 'r', offset=offset,
                             bytes=bytes_to_send) as fp:
            mp.upload_part_from_file(fp, part_num=i + 1)

    # Finish the upload
    mp.complete_upload()

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
    pass

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

    project_file = os.path.abspath(args.project_file).replace('/drives/d/', 'D:/').replace('/home/mobaxterm/d/', 'D:/')

    project_name = get_project_name(project_file)
    files = get_files(project_name)

    if check_files_ok(files):
        platform_name = get_platform_name()
        info_filename = get_project_infofile(project_name)

        zipfilename = get_zipfilename_from_infofile(info_filename)
        zip_root_folder = get_output_root_folder()

        print 'Start zipping files to %s' % zipfilename
        zipfiles(files, zip_root_folder, zipfilename)

        print 'Start uploading file %s' % zipfilename
        upload(bucket_name, zipfilename)

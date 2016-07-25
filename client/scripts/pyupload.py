from ue4config import conf
# Use python to upload files to server
def upload_s3(bucket_name, filename):
    '''
    Upload file to Amazon S3 server
    '''
    import boto
    from filechunkio import FileChunkIO
    # From http://boto.cloudhackers.com/en/latest/s3_tut.html
    # from boto.s3.connection import S3Connection
    S3_ACCESS_KEY = os.environ['AWS_ACCESS_KEY_ID']
    S3_SECRET_KEY = os.environ['AWS_SECRET_ACCESS_KEY']
    # conn = S3Connection(S3_ACCESS_KEY, S3_SECRET_KEY)
    print S3_ACCESS_KEY
    print S3_SECRET_KEY

    # Connect to S3
    c = boto.connect_s3(host='s3-ap-northeast-1.amazonaws.com')
    # c = boto.connect_s3(host='s3-us-west-1.amazonaws.com')
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

def upload_scp(filename):
    import scp, os
    scp_conf = conf['scp']
    client = scp.Client(host = scp_conf['host'], user = scp_conf['user'], password = scp_conf['password'])
    basefilename = os.path.basename(filename)
    remote_filename = os.path.join(scp_conf['rootdir'], basefilename)
    print 'Try to do scp %s %s' % (filename, remote_filename)
    client.transfer(filename, remote_filename)

if __name__ == '__main__':
    upload_scp("pyupload.py")

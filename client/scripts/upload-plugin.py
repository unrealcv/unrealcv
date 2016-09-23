import argparse, os
import uploadutil, gitutil, ue4util

if __name__ == '__main__':
    # Copy binaries to a remote server with scp
    # Upload built plugin to upload location
    parser = argparse.ArgumentParser()
    parser.add_argument('remote')
    parser.add_argument('--version')
    parser.add_argument('--password', help = 'Account password')
    parser.add_argument('--pkey', help = 'Private key')

    cur_dir = os.path.dirname(os.path.abspath(__file__))

    args = parser.parse_args()
    if args.version:
        plugin_version = args.version
    else:
        plugin_version = gitutil.get_short_version(cur_dir)
    remote = args.remote
    # remote = '%s@%s:%s'

    plugin_output_folder = ue4util.get_real_abspath(os.path.join(cur_dir, 'built_plugin/%s' % plugin_version))
    if not os.path.isdir(plugin_output_folder):
        exit('Version %s of plugin does not exist, compile first' % plugin_version)

    print 'Upload plugin from %s -> %s' % (repr(plugin_output_folder), repr(remote))
    # Use repr so that I can correctly see the difference between / and \\
    local_root = os.path.join(cur_dir, 'built_plugin')
    uploadutil.upload_scp(remote, [plugin_output_folder], local_root,
        password=args.password,
        key_filename=args.pkey,
        DEBUG=False)
    # uploadutil.upload_scp()

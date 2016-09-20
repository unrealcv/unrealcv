import argparse, os
import uploadutil, gitutil, ue4util

if __name__ == '__main__':
    # Copy binaries to a remote server with scp
    # Upload built plugin to upload location
    parser = argparse.ArgumentParser()
    parser.add_argument('remote')
    parser.add_argument('--version')

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

    print 'Upload plugin from %s -> %s' % (plugin_output_folder, remote)
    uploadutil.upload_scp(remote, [plugin_output_folder], 'built_plugin')
    # uploadutil.upload_scp()

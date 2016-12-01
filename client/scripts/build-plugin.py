'''
Package ue4 plugin and release it to output location defined in ue4config.py
'''
import os, sys, json, pdb, argparse
import ue4util, gitutil, shutil

def is_valid(plugin_file):
    plugin_folder = os.path.dirname(plugin_file)

    if gitutil.is_dirty(plugin_folder):
        print 'Error: uncommited changes of this repo exist'
        return False
    return True


def build_plugin(plugin_file, plugin_output_folder):
    '''
    Build plugin binaries
    Return whether the operation is successful
    '''
    UAT_script = ue4util.get_UAT_script()
    # cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket -targetplatforms=Win64+Linux' % (UAT_script, plugin_file, plugin_output_folder)
    # cmd = '%s BuildPlugin ' % UAT_script
    import subprocess
    cmd = [UAT_script, 'BuildPlugin', '-plugin=%s' % plugin_file, '-package=%s' % plugin_output_folder, '-rocket', '-targetplatforms=Win64+Linux']
    subprocess.call(cmd)
    # ue4util.run_ue4cmd(cmd)

    # Post processing clean up intermediate files
    intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
    print 'Delete intermediate folder %s' % intermediate_folder
    shutil.rmtree(intermediate_folder)


def save_version_info(info_filename, plugin_file):
    ''' Save version info of UnrealCV plugin '''
    plugin_folder = os.path.dirname(plugin_file)
    plugin_version = gitutil.get_short_version(plugin_folder)
    info = dict(
        plugin_version = plugin_version
    )
    print info
    with open(info_filename, 'w') as f:
        json.dump(info, f, indent = 4)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--dev', help='Continue doing build without checking git status', action='store_true')
    args = parser.parse_known_args()[0]

    # Files is relative to this python script
    cur_dir = os.path.dirname(os.path.abspath(__file__))

    plugin_file = ue4util.get_real_abspath(os.path.join(cur_dir, '../../UnrealCV.uplugin'))
    if args.dev:
        plugin_output_folder = os.path.join(cur_dir, 'tmp')
        build_plugin(plugin_file, plugin_output_folder)
    else:
        if not is_valid(plugin_file):
            return False

        plugin_version = gitutil.get_short_version(cur_dir)
        plugin_output_folder = ue4util.get_real_abspath(os.path.join(cur_dir, 'built_plugin/%s' % plugin_version))
        ue4util.mkdirp(plugin_output_folder)
        info_filename = os.path.join(plugin_output_folder, 'unrealcv-info.txt')

        # Build plugin to disk
        build_plugin(plugin_file, plugin_output_folder)
        save_version_info(info_filename, plugin_file)


if __name__ == '__main__':
    main()

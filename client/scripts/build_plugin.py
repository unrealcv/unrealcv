'''
Package ue4 plugin and release it to output location defined in ue4config.py
This script is tested in mac, linux and windows (under cygwin)
'''
import os, sys, json, pdb, argparse
import ue4util, gitutil, shutil
plugin_ext = '.uplugin'

def is_valid(plugin_file):
    plugin_folder = os.path.dirname(os.path.abspath(plugin_file))

    if gitutil.is_git_dirty(plugin_folder):
        print 'Error: uncommited changes of this repo exist'
        return False
    return True

def build(plugin_file, engine_path, plugin_output_folder):
    '''
    Build plugin binaries
    Return whether the operation is successful
    '''
    UAT_script = ue4util.get_UAT_script(engine_path)
    import subprocess

    ue4_plugin_file = ue4util.get_real_abspath(plugin_file)
    ue4_plugin_output_folder = ue4util.get_real_abspath(plugin_output_folder)
    # Need to convert python abs path to what UE4 can recognize
    # This is especially important for windows and cygwin
    cmd = [UAT_script, 'BuildPlugin', '-plugin=%s' % ue4_plugin_file, '-package=%s' % ue4_plugin_output_folder, '-rocket', '-targetplatforms=Win64+Linux+Mac']
    subprocess.call(cmd)

    # Post process: clean up intermediate files and make it suitable for release
    intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
    print 'Delete intermediate folder %s' % intermediate_folder
    shutil.rmtree(intermediate_folder)

def save_version_info(info_filename, plugin_file):
    ''' Save git sha version info of plugin '''
    plugin_folder = os.path.dirname(plugin_file)
    sha_version = gitutil.get_sha_version(plugin_folder)
    tag_version = gitutil.get_git_tag(plugin_folder)
    info = dict(
        sha_version = sha_version,
        tag_version = tag_version
    )
    print info
    with open(info_filename, 'w') as f:
        json.dump(info, f, indent = 4)

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument('--plugin', required = True, help='The plugin file to build')
    parser.add_argument('--engine', required = True, help='The Unreal Engine install path')
    parser.add_argument('--output', help='Where to put the artifact')
    parser.add_argument('--dev', action='store_true', help='Continue doing build without checking git status')
    parser.add_argument('--release', action='store_true', help='Make sure this commit is properly tagged, so that we can release it')

    args = parser.parse_args()
    build_plugin(args.plugin, args.engine, args.output, args.dev, args.release)

def build_plugin(plugin, engine, output="", dev=False, release=False):
    cur_dir = os.path.abspath(os.path.curdir)
    plugin_file = os.path.abspath(plugin)

    if not plugin_file.endswith(plugin_ext):
        print 'Plugin file %s is not with file extension %s' % (plugin_file, plugin_ext)
        return False

    plugin_folder = os.path.dirname(plugin_file)
    tag_version = gitutil.get_git_tag(plugin_folder)
    sha_version = gitutil.get_sha_version(plugin_folder)

    if output:
        # Manually specify the output folder
        plugin_output_folder = output
    elif dev:
        # No version info is required
        plugin_output_folder = os.path.join(cur_dir, 'tmp')
    else:
        # Compute output folder based on tag or sha (tag not present)
        plugin_version = tag_version if tag_version else sha_version
        plugin_output_folder = ue4util.get_real_abspath(os.path.join(cur_dir, 'built_plugin/%s/unrealcv' % plugin_version))

    if dev:
        # No need to do any check, just make sure it can be compiled
        build(plugin_file, plugin_output_folder)
    else:
        if not is_valid(plugin_file):
            return False

        if release and tag_version == '':
            print 'Please properly tag the project, so that it can be released'
            return False

        ue4util.mkdirp(plugin_output_folder)
        info_filename = os.path.join(plugin_output_folder, 'plugin-info.txt')

        # Build plugin to disk
        build(plugin_file, engine, plugin_output_folder)
        save_version_info(info_filename, plugin_file)



if __name__ == '__main__':
    main()

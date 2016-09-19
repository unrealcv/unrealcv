import argparse, shutil, os
import ue4util, gitutil
def install_plugin(project_file, plugin_folder):
    project_folder = os.path.dirname(project_file)
    install_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')
    if os.path.isdir(install_folder):
        shutil.rmtree(install_folder) # Complete remove old version, a little dangerous

    shutil.copytree(plugin_folder, install_folder)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--version')
    parser.add_argument('project_file')
    args = parser.parse_args()
    cur_dir = os.path.dirname(os.path.abspath(__file__))

    if args.version:
        plugin_version = args.version
    elif not gitutil.is_dirty(cur_dir):
        plugin_version = gitutil.get_short_version('.')
    else:
        exit('Uncommited changes of plugin exist, can not get current version')
    project_file = ue4util.get_real_abspath(args.project_file)

    cur_dir = os.path.dirname(os.path.abspath(__file__))
    plugin_folder = os.path.join(cur_dir, 'built_plugin/%s' % plugin_version)
    install_plugin(project_file, plugin_folder)

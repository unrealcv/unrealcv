import argparse, shutil, os
import ue4util
def install_plugin(project_file, plugin_folder):
    project_folder = os.path.dirname(project_file)
    install_folder = os.path.join(project_folder, 'Plugins', 'unrealcv')
    if os.path.isdir(install_folder):
        shutil.rmtree(install_folder) # Complete remove old version, a little dangerous

    shutil.copytree(plugin_folder, install_folder)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('plugin_version')
    parser.add_argument('project_file')
    args = parser.parse_args()

    plugin_version = args.plugin_version
    project_file = ue4util.get_real_abspath(args.project_file)

    cur_dir = os.path.dirname(os.path.abspath(__file__))
    plugin_folder = os.path.join(cur_dir, 'built_plugin/%s' % plugin_version)
    install_plugin(project_file, plugin_folder)

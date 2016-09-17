'''
Package ue4 plugin and release it to output location defined in ue4config.py
'''
import os, sys, json, pdb, argparse
import ue4util, gitutil, shutil, uploadutil

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
    bin = UAT_script.replace(' ', '\ ')
    cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket -targetplatforms=Win64+Linux' % (bin, plugin_file, plugin_output_folder)
    ue4util.run_ue4cmd(cmd)

    # Post processing clean up intermediate files
    intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
    print 'Delete intermediate folder %s' % intermediate_folder
    shutil.rmtree(intermediate_folder)

def upload_plugin(plugin_output_folder, upload_conf):
    type = upload_conf['Type']
    upload_handlers = dict(
        scp = uploadutil.upload_scp,
        s3 = uploadutil.upload_s3,
    )
    upload_handlers[type](upload_conf, [plugin_output_folder], 'built_plugin')

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
    # Files is relative to this python script
    cur_dir = os.path.dirname(os.path.abspath(__file__))

    plugin_file = ue4util.get_real_abspath(os.path.join(cur_dir, '../../UnrealCV.uplugin'))
    if not is_valid(plugin_file):
        return False

    plugin_version = gitutil.get_short_version('.')
    plugin_output_folder = ue4util.get_real_abspath(os.path.join(cur_dir, 'built_plugin/%s' % plugin_version))
    ue4util.mkdirp(plugin_output_folder)
    info_filename = os.path.join(plugin_output_folder, 'unrealcv-info.txt')

    # Build plugin to disk
    build_plugin(plugin_file, plugin_output_folder)
    save_version_info(info_filename, plugin_file)

    # Upload built plugin to upload location
    # upload_confs = ue4config.conf['PluginOutput']
    # for upload_conf in upload_confs:
    #     upload_plugin(plugin_output_folder, upload_conf)

if __name__ == '__main__':
    main()

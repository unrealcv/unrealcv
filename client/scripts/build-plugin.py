'''
Package ue4 plugin and release it to output location defined in ue4config.py
'''
import os, sys
import ue4config
import ue4util, gitutil, shutil, uploadutil

def build_plugin(plugin_file, plugin_output_folder):
    '''
    Build plugin binaries
    Return whether the operation is successful
    '''
    UAT_script = ue4config.conf['UATScript']
    if not os.path.isfile(UAT_script):
        print('Can not find Automation Script of UE4 %s' % UAT_script)
        print('Please set UnrealEnginePath in ue4config.py correctly first')
        return False
    else:
        if gitutil.is_dirty('.'):
            print 'Error: uncommited changes of this repo exist'
            return False

        UAT_script = UAT_script.replace(' ', '\ ')
        cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket -targetplatforms=Win64+Linux' % (UAT_script, plugin_file, plugin_output_folder)
        ue4util.run_ue4cmd(cmd)

        # Post processing clean up intermediate files
        intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
        print 'Delete intermediate folder %s' % intermediate_folder
        shutil.rmtree(intermediate_folder)
        return True

def upload_plugin(plugin_output_folder, upload_conf):
    type = upload_conf['Type']
    upload_handlers = dict(
        scp = uploadutil.upload_scp,
        s3 = uploadutil.upload_s3,
    )
    upload_handlers[type](upload_conf, [plugin_output_folder], '.')

def save_version_info(info_filename, plugin_file):
    plugin_folder = os.path.dirname(plugin_file)
    if gitutil.is_dirty(plugin_folder):
        return False

    ''' Save version info of UnrealCV plugin '''
    plugin_version = gitutil.get_short_version(plugin_folder)
    info = dict(
        plugin_version = plugin_version
    )

    with open(info_filename, 'w') as f:
        json.dumps(info, f, indent = 4)
    return True

if __name__ == '__main__':
    plugin_version = gitutil.get_short_version('.')
    plugin_file = ue4util.get_real_abspath('../../UnrealCV.uplugin')
    plugin_output_folder = ue4util.get_real_abspath('./built_plugin/%s' % plugin_version)
    info_filename = os.path.join(plugin_output_folder, 'unrealcv-info.txt')

    # Build plugin to disk
    is_built = False
    if save_version_info(info_filename, plugin_file):
        build_plugin(plugin_file, plugin_output_folder)
        is_built = True

    # Upload built plugin to upload location
    if is_built:
        upload_confs = ue4config.conf['PluginOutput']
        for upload_conf in upload_confs:
            upload_plugin(plugin_output_folder, upload_conf)

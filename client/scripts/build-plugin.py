import os, sys
import ue4config
import ue4util, gitutil, shutil

def build_plugin():
    UAT_script = conf['UATScript']
    if not os.path.isfile(UAT_script):
        print('Can not find Automation Script of UE4 %s' % UAT_script)
        print('Please set UnrealEnginePath in ue4config.py correctly first')
    else:
        if gitutil.is_dirty(os.path.abspath('.')):
            print 'Error: uncommited changes of this repo exist'
            return False

        plugin_version = gitutil.get_short_version('.')
        plugin_file = os.path.abspath('../../UnrealCV.uplugin')
        if sys.platform == 'cygwin':
            plugin_file = ue4util.cygwinpath_to_winpath(plugin_file)

        plugin_output_folder = os.path.abspath('./unrealcv-%s' % plugin_version)
        if sys.platform == 'cygwin':
            output_folder = ue4util.cygwinpath_to_winpath(output_folder)

        UAT_script = UAT_script.replace(' ', '\ ')
        cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket' % (UAT_script, plugin_file, plugin_output_folder)
        print(cmd)
        os.system(cmd)

        # Clean up intermediate files
        intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
        shutil.rmtree(intermediate_folder)

def output_plugin(output_conf):
    output_conf['type']
    upload_handlers = dict(
        scp = upload_scp,
        s3 = upload_s3,
    )
    pass

if __name__ == '__main__':
    # build_plugin()
    output_confs = ue4config.conf['PluginOutput']
    for conf in output_confs:
        print conf['type']
        output_plugin(conf)

import os, sys
import ue4config
import ue4util, gitutil, shutil, uploadutil

plugin_version = gitutil.get_short_version('.')
plugin_output_folder = os.path.abspath('./unrealcv-%s' % plugin_version)

def build_plugin():
    UAT_script = ue4config.conf['UATScript']
    if not os.path.isfile(UAT_script):
        print('Can not find Automation Script of UE4 %s' % UAT_script)
        print('Please set UnrealEnginePath in ue4config.py correctly first')
        return False
    else:
        if gitutil.is_dirty(os.path.abspath('.')):
            print 'Error: uncommited changes of this repo exist'
            return False

        plugin_file = os.path.abspath('../../UnrealCV.uplugin')
        if sys.platform == 'cygwin':
            plugin_file = ue4util.cygwinpath_to_winpath(plugin_file)

        UAT_script = UAT_script.replace(' ', '\ ')
        cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket' % (UAT_script, plugin_file, plugin_output_folder)
        print(cmd)
        os.system(cmd)

        # Clean up intermediate files
        intermediate_folder = os.path.join(plugin_output_folder, 'Intermediate')
        shutil.rmtree(intermediate_folder)
        return True

def output_plugin(output_conf):
    type = output_conf['Type']
    upload_handlers = dict(
        scp = uploadutil.upload_scp,
        s3 = uploadutil.upload_s3,
    )
    upload_handlers[type](output_conf, [plugin_output_folder], '.')

if __name__ == '__main__':
    if build_plugin():
        output_confs = ue4config.conf['PluginOutput']
        for conf in output_confs:
            print conf['Type']
            output_plugin(conf)

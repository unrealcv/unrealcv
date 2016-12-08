'''
Package ue4 plugin and release it to output location defined in ue4config.py
'''
import os, sys, json, pdb, argparse
import ue4util, gitutil, shutil

class UE4PluginUtil(ue4util.UE4Util):
    def build_plugin(self, plugin_file, output_folder):
        '''
        Build plugin binaries
        '''
        import subprocess
        UAT_script = self.get_UAT_script()
        cmd = [UAT_script, 'BuildPlugin', '-plugin=%s' % plugin_file, '-package=%s' % output_folder, '-rocket', '-targetplatforms=Win64+Linux+Mac']
        subprocess.call(cmd)

        # Clean up intermediate files, this is required by UE marketplace policy
        intermediate_folder = os.path.join(output_folder, 'Intermediate')
        print 'Delete intermediate folder %s' % intermediate_folder
        shutil.rmtree(intermediate_folder)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--engine_path', help='The folder that contains Unreal Engine 4')
    args = parser.parse_args()

    # Files are relative to this python script
    cur_dir = os.path.dirname(os.path.abspath(__file__))
    plugin_file = ue4util.get_real_abspath(os.path.join(cur_dir, '../../UnrealCV.uplugin'))

    output_folder = os.path.join(cur_dir, '../../built')

    util = UE4PluginUtil(args.engine_path)
    util.build_plugin(plugin_file, output_folder)

if __name__ == '__main__':
    main()

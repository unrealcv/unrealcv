# Build script of unrealcv, supports win, linux and mac.
# Weichao Qiu @ 2017
import argparse

def main():
    # Parse arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('--install', action='store_true')
    args = parser.parse_args()
    need_install = args.install

    ue4 = UE4Automation()

    # Build the plugin
    abs_unrealcv_descriptor = os.path.abspath('UnrealCV.uplugin')
    abs_output_folder = os.path.abspath('Plugins/UnrealCV')
    ue4.build_plugin(abs_unrealcv_descriptor, abs_output_folder)

    # Install the plugin if requested
    if need_install:
        ue4.install()

import subprocess, sys, os, argparse, platform, logging, glob, shutil
try: input = raw_input
except NameError: pass
class UE4Automation:
    ''' UE4 engine wrapper '''
    def __init__(self):
        self.platform_name = self._get_platform_name()
        self.UE4_dir = self._get_UE4_dir()
        self.abs_UAT_path = self._get_UATPath()

    def build_plugin(self, plugin_descriptor, output_folder):
        abs_plugin_descriptor = os.path.abspath(plugin_descriptor)
        abs_output_folder = os.path.abspath(output_folder)

        if os.path.isdir(abs_output_folder):
            print('Output folder "%s" already exists, skip compilation.' % abs_output_folder)
            print('Remove this folder if you want to compile the plugin with a different UE4 version.')
        else:
            script_dir = os.path.dirname(os.path.realpath(__file__))
            subprocess.call([
                self.abs_UAT_path, 'BuildPlugin',
                '-plugin=%s' % abs_plugin_descriptor,
                '-package=%s' % abs_output_folder,
                '-rocket', '-targetplatforms=%s' % self.platform_name
            ], cwd = script_dir)

    def install(self):
        ''' Install the plugin to UE4 engine folder '''
        print('-' * 30 + ' Install ' + '-' * 30)
        engine_plugin_folder = os.path.join(self.UE4_dir, 'Engine', 'Plugins')
        abs_tgt_unrealcv_folder = os.path.join(engine_plugin_folder, 'UnrealCV')
        abs_src_unrealcv_folder = os.path.abspath('Plugins/UnrealCV')

        if os.path.isdir(abs_tgt_unrealcv_folder):
            print('UnrealCV is already found in the Engine/Plugins folder, do you want to overwrite')
            print('Type yes or y to continue:')
            choice = input()
            if choice.lower() not in ['y', 'yes']:
                print('Installation cancelled')
                return
            else:
                shutil.rmtree(abs_tgt_unrealcv_folder)

        print('Copy the plugin from %s to %s' % (abs_src_unrealcv_folder, abs_tgt_unrealcv_folder))
        shutil.copytree(abs_src_unrealcv_folder, abs_tgt_unrealcv_folder)
        print('Installation of UnrealCV is successful.')

    def package(self, project_path, output_folder):
        abs_project_path = os.path.abspath(project_path)
        abs_output_folder = os.path.abspath(output_folder)
        subprocess.call([
            self.abs_UAT_path, 'BuildCookRun',
            '-project=%s' % abs_project_path,
            '-archivedirectory=%s' % abs_output_folder,
            '-platform=%s' % self.platform_name,
            '-clientconfig=Development', '-serverconfig=Development',
            '-noP4', '-allmaps', '-stage', '-pak', '-archive', '-cook', '-build'
        ])

    def _get_UATPath(self):
        platform2UATRelativePath = {
            'Linux': 'Engine/Build/BatchFiles/RunUAT.sh',
            'Mac': 'Engine/Build/BatchFiles/RunUAT.sh',
            'Win64': 'Engine\\Build\\BatchFiles\\RunUAT.bat'
        }
        platform_name = self._get_platform_name()
        UAT_relative_path = platform2UATRelativePath.get(platform_name)
        UAT_abs_path = os.path.join(self.UE4_dir, UAT_relative_path)
        return UAT_abs_path


    def _get_platform_name(self):
        '''' Python and UE4 use different names for platform, in this script we will use UE4 platform name exclusively '''
        py2UE4 = {"Darwin": "Mac", "Windows": "Win64", "Linux": "Linux"}
        # Key: python platform name, Value: UE4
        platform_name = py2UE4.get(platform.system())
        if not platform_name:
            print('Can not recognize platform %s' % platform.system())
        return platform_name

    def _get_UE4_dir(self):
        win_candidates = [
            'C:\Program Files\Epic Games\UE_4.??',
            'D:\Program Files\Epic Games\UE_4.??',]
        linux_candidates = [
            os.path.expanduser('~/UnrealEngine'),
            os.path.expanduser('~/workspace/UnrealEngine'),
            os.path.expanduser('~/workspace/UE4??'),]
        mac_candidates = ['/Users/Shared/Epic Games/UE_4.??',]
        search_candidates = {'Linux': linux_candidates, 'Mac': mac_candidates, 'Win64': win_candidates}
        candidates = search_candidates.get(self._get_platform_name())

        found_UE4 = []
        for c in candidates: found_UE4 += glob.glob(c)
        # Ask user to make a selection
        if len(found_UE4) == 1: return found_UE4[0]
        if len(found_UE4) == 0:
            print('Can not automatically found a UE4 path, please specify it with --UE4')

        print('Found UE4 in the following path, please make a selection:')
        print('\n'.join('%d : %s' % (i+1, found_UE4[i]) for i in range(len(found_UE4))))

        num = int(input())
        return found_UE4[num-1]

if __name__ == '__main__':
    main()

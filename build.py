# Build script of unrealcv, supports win, linux and mac.
# Weichao Qiu @ 2017

import subprocess, sys, os, argparse, platform, logging, glob, shutil
try: input = raw_input
except NameError: pass
logger = logging.getLogger(__name__)

# Python and UE4 use different names for platform, in this script we will use UE4 platform name exclusively
def get_platform_name():
    py2UE4 = {"Darwin": "Mac", "Windows": "Win64", "Linux": "Linux"}
    # Key: python platform name, Value: UE4
    platform_name = py2UE4.get(platform.system())
    if not platform_name:
        logger.error('Can not recognize platform %s' % platform.system())
    return platform_name

def UATPath(UE4_dir):
    platform2UATRelativePath = {
        'Linux': 'Engine/Build/BatchFiles/RunUAT.sh',
        'Mac': 'Engine/Build/BatchFiles/RunUAT.sh',
        'Win64': 'Engine\\Build\\BatchFiles\\RunUAT.bat'
    }
    platform_name = get_platform_name()
    UAT_relative_path = platform2UATRelativePath.get(platform_name)
    UAT_abs_path = os.path.join(UE4_dir, UAT_relative_path)
    return UAT_abs_path

def UE4_discover():
    win_candidates = [
        'C:\Program Files\Epic Games\UE_4.??',
        'D:\Program Files\Epic Games\UE_4.??',]
    linux_candidates = [
        os.path.expanduser('~/UnrealEngine'), os.path.expanduser('~/workspace/UnrealEngine'),]
    mac_candidates = []
    search_candidates = {'Linux': linux_candidates, 'Mac': mac_candidates, 'Win64': win_candidates}
    candidates = search_candidates.get(get_platform_name())

    found_UE4 = []
    for c in candidates: found_UE4 += glob.glob(c)
    # Ask user to make a selection
    if len(found_UE4) == 1: return found_UE4[0]
    if len(found_UE4) == 0:
        logger.info('Can not automatically found a UE4 path, please specify it with --UE4')

    print('Found UE4 in the following path, please make a selection:')
    print('\n'.join('%d : %s' % (i+1, found_UE4[i]) for i in range(len(found_UE4))))

    num = int(input())
    return found_UE4[num-1]

def install(UE4_dir):
    engine_plugin_folder = os.path.join(UE4_dir, 'Engine', 'Plugins')
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

    shutil.copytree(abs_src_unrealcv_folder, abs_tgt_unrealcv_folder)
    print('Installation of UnrealCV is successful.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--UE4')
    parser.add_argument('--install', action='store_true')

    args = parser.parse_args()
    need_install = args.install

    # TODO: Try to get UE4 path from the argument or from os.environ first
    UE4_dir = UE4_discover()
    abs_UAT_path = os.path.abspath(UATPath(UE4_dir))

    # unrealcv_folder = os.environ.get('UnrealCV')
    unrealcv_descriptor = 'UnrealCV.uplugin'
    abs_unrealcv_descriptor = os.path.abspath(unrealcv_descriptor)
    abs_output_folder = os.path.abspath('Plugins/UnrealCV')
    platform_name = get_platform_name()
    script_dir = os.path.dirname(os.path.realpath(__file__))

    subprocess.call([
        abs_UAT_path, 'BuildPlugin',
        '-plugin=%s' % abs_unrealcv_descriptor,
        '-package=%s' % abs_output_folder,
        '-rocket', '-targetplatforms=%s' % platform_name
    ], cwd = script_dir)

    if need_install:
        install(UE4_dir)

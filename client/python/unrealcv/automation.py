# Build script of unrealcv, supports win, linux and mac.
# A single file library
# Weichao Qiu @ 2017
import subprocess, sys, os, argparse, platform, logging, glob, shutil, json
try: input = raw_input # to support python3
except NameError: pass

def get_platform_name():
    ''''
    Python and UE4 use different names for platform, in this script we will use UE4 platform name exclusively
    '''
    py2UE4 = {
        # pyname : ue4name
        "Darwin": "Mac",
        "Windows": "Win64",
        "Linux": "Linux"
    }
    # Key: python platform name, Value: UE4
    platform_name = py2UE4.get(platform.system())
    if not platform_name:
        print('Can not recognize platform %s' % platform.system())
    return platform_name

def get_plugin_version(plugin_descriptor):
    with open(plugin_descriptor) as f:
        description = json.load(f)
    plugin_version = description['VersionName']
    return plugin_version

class UE4Automation:
    ''' UE4 engine wrapper '''
    def __init__(self, engine):
        self.platform_name = self._get_platform_name()
        if engine:
            self.UE4_dir = engine
        else:
            self.UE4_dir = self._get_UE4_dir()
        self.abs_UAT_path = self._get_UATPath()

        if not os.path.isfile(self.abs_UAT_path):
            print('Can not found UAT script in the engine folder %s' % self.UE4_dir)

    def _get_platform_name(self):
        return get_platform_name()

    def build_plugin(self, plugin_descriptor, output_folder, overwrite = False):
        '''
        Use RunUAT script to build the plugin

        Parameters
        ----------
        plugin_descriptor : str
        output_folder : str
        overwrite : bool
            Whether the compiled binary folder should be overwriten?
        '''
        abs_plugin_descriptor = os.path.abspath(plugin_descriptor)
        abs_output_folder = os.path.abspath(output_folder)

        if overwrite == False and os.path.isdir(abs_output_folder):
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

    def install(self, plugin_folder, overwrite = False):
        '''
        Install the plugin to UE4 engine folder

        Parameters
        ----------
        plugin_folder : str
            The plugin folder with compiled binaries
        '''
        print('-' * 30 + ' Install ' + '-' * 30)
        engine_plugin_folder = os.path.join(self.UE4_dir, 'Engine', 'Plugins')
        abs_tgt_unrealcv_folder = os.path.join(engine_plugin_folder, 'UnrealCV')
        abs_src_unrealcv_folder = plugin_folder

        if os.path.isdir(abs_tgt_unrealcv_folder):
            if overwrite:
                shutil.rmtree(abs_tgt_unrealcv_folder)
            else:
                print('UnrealCV is already found in the Engine/Plugins folder')
                return

        print('Copy the plugin from %s to %s' % (abs_src_unrealcv_folder, abs_tgt_unrealcv_folder))
        shutil.copytree(abs_src_unrealcv_folder, abs_tgt_unrealcv_folder)
        print('Installation of UnrealCV is successful.')

    def package(self, project_descriptor, output_folder, overwrite = False):
        '''
        Package an UE4 project

        Parameters
        ----------
        project_descriptor : str
            UE4 project file name ends with *.uproject
        overwrite : bool
            Overwrite existing files
        '''

        abs_project_path = os.path.abspath(project_descriptor)
        abs_output_folder = os.path.abspath(output_folder)

        if overwrite == False and os.path.isdir(abs_output_folder):
            print('Packaged binary already exist')
        else:
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



    def _get_UE4_dir(self):
        win_candidates = [
            'C:\\Program Files\\Epic Games\\UE_4.??',
            'D:\\Program Files\\Epic Games\\UE_4.??',]
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


def UE4Binary(binary_path):
    '''
    Return a platform-dependent binary for user.

    Examples
    --------
    >>> binary = UE4Binary('./WindowsNoEditor/RealisticRendering.exe')  # For windows
    >>> binary = UE4Binary('./LinuxNoEditor/RealisticRendering/Binaries/RealisticRendering') # For Linux
    >>> binary = UE4Binary('./MacNoEditor/RealisticRendering.app') # For mac
    >>> with binary: # Automatically launch and close the binary
    >>>     client.request('vget /unrealcv/status')
    >>> # Or alternatively
    >>> binary.start()
    >>> client.request('vget /unrealcv/status')
    >>> binary.close()
    '''
    binary_wrapper_selection = {
        'Linux': LinuxBinary,
        'Mac': MacBinary,
        'Win64': WindowsBinary,
    }
    platform_name = get_platform_name()
    binary_wrapper_class = binary_wrapper_selection.get(platform_name)
    if binary_wrapper_class:
        return binary_wrapper_class(binary_path)
    else:
        # Add error handling if needed
        return None

import time, os
# The environment runner
class UE4BinaryBase(object):
    '''
    UE4BinaryBase is the base class for all platform-dependent classes, it is different from UE4Binary which serves as a factory to create a platform-dependent binary wrapper. User should use UE4Binary instead of UE4BinaryBase

    Binary is a python wrapper to control the start and stop of a UE4 binary.
    The wrapper provides simple features to start and stop the binary, mainly useful for automate the testing.

    Usage:
        bin = UE4Binary('/tmp/RealisticRendering/RealisticRendering')
        with bin:
            client.request('vget /camera/0/lit test.png')
    '''
    def __init__(self, binary_path):
        self.binary_path = binary_path

    def __enter__(self):
        ''' Start the binary '''
        if os.path.isfile(self.binary_path) or os.path.isdir(self.binary_path):
            self.start()
        else:
            print('Binary %s can not be found' % self.binary_path)

    def __exit__(self, type, value, traceback):
        ''' Close the binary '''
        self.close()

class WindowsBinary(UE4BinaryBase):
    def start(self):
        print('Start windows binary %s' % self.binary_path)
        subprocess.Popen(self.binary_path)
        time.sleep(10) # FIXME: How long is needed for the binary to launch?
        # Wait for the process to run. FIXME: Wait for an output line?

    def close(self):
        # Kill windows process
        basename = os.path.basename(self.binary_path)
        cmd = ['taskkill', '/F', '/IM', basename]
        print('Kill windows binary with command %s' % cmd)
        subprocess.call(cmd)

class LinuxBinary(UE4BinaryBase):
    def start(self):
        null_file = open(os.devnull, 'w')
        popen_obj = subprocess.Popen([self.binary_path], stdout = null_file, stderr = null_file)
        self.pid = popen_obj.pid
        time.sleep(6)

    def close(self):
        # Kill Linux process
        cmd = ['kill', str(self.pid)]
        print('Kill process %s with command %s' % (self.pid, cmd))
        subprocess.call(cmd)

class MacBinary(UE4BinaryBase):
    def start(self):
        popen_obj = subprocess.Popen([
            'open',
            self.binary_path
        ])
        self.program_name = os.path.basename(self.binary_path).replace('.app', '')
        # TODO: Track the stdout to see whether it is started?
        time.sleep(5)

    def close(self):
        subprocess.call(['pkill', self.program_name])

class DockerBinary(UE4BinaryBase):
    def start(self):
        # nvidia-docker run --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" qiuwch/rr:${version} > log/docker-rr.log &
        pass

    def close(self):
        pass

if __name__ == '__main__':
    import argparse
    from unrealcv import client

    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', help = 'Test running the binary', required = True)
    # Example: D:\temp\dev_project_output\WindowsNoEditor\UnrealcvDevProject.exe

    args = parser.parse_args()
    # A hacky way to determine the binary type
    binary_path = args.binary
    if binary_path.lower().endswith('.exe'):
        binary = WindowsBinary(binary_path)
    elif binary_path.lower().endswith('.app'):
        binary = MacBinary(binary_path)
    else:
        binary = LinuxBinary(binary_path)
    with binary:
        client.connect()
        client.request('vget /unrealcv/status')

    pass
    # Try some simple tests in here?

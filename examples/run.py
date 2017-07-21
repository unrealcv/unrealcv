#!env python
import argparse, sys, os, subprocess, platform, time

# The environment runner
class BinaryRunner(object):
    ''' Base class for binary '''
    def __init__(self, binary_path):
        self.binary_path = binary_path

    def __enter__(self):
        ''' Start the binary '''
        if os.path.isfile(self.binary_path) or os.path.isdir(self.binary_path):
            self.start()
        else:
            print('Binary %s can not be found' % self.binary_path)
            sys.exit(-1)

    def __exit__(self, type, value, traceback):
        ''' Close the binary '''
        self.close()

class WindowsBinaryRunner(BinaryRunner):
    def start(self):
        # self.exe = self.project_name + '.exe'
        # self.win_binary = os.path.join(self.unzip_folder, self.exe)
        print('Start windows binary %s' % self.binary_path)
        # subprocess.call(self.win_binary)
        subprocess.Popen(self.binary_path)
        time.sleep(10)
        # Wait for the process to run. FIXME: Wait for an output line?

    def close(self):
        # Kill windows process
        # See this https://github.com/qiuwch/unrealcv-packaging/blob/master/windows/test-rr.bat
        basename = os.path.basename(self.binary_path)
        cmd = ['taskkill', '/F', '/IM', basename]
        print('Kill windows binary with command %s' % cmd)
        subprocess.call(cmd)

class LinuxBinaryRunner(BinaryRunner):
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

class MacBinaryRunner(BinaryRunner):
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

class DockerBinaryRunner(BinaryRunner):
    # Not implemented yet
    def start(self):
        # nvidia-docker run --rm -p 9000:9000 --env="DISPLAY" --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" qiuwch/rr:${version} > log/docker-rr.log &
        pass

    def close(self):
        pass

def run(binary_path, script_path):
    system_name = platform.system()
    if system_name == 'Windows':
        binary = WindowsBinaryRunner(binary_path)
    elif system_name == 'Linux':
        binary = LinuxBinaryRunner(binary_path)
    elif system_name == 'Darwin':
        binary = MacBinaryRunner(binary_path)

    with binary:
        subprocess.call(
            ['python', script_path]
        )

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('script', help='The script to run')
    parser.add_argument('--binary', help='The binary to run',
        default = './RealisticRendering-Linux-0.3.9/LinuxNoEditor/RealisticRendering/Binaries/Linux/RealisticRendering'
    )

    args = parser.parse_args()
    binary_path = args.binary
    script_path = args.script
    run(binary_path, script_path)

# Build UnrealCV against different UE4 version to check compatibility
import subprocess, os, sys

def format_cmd(UE4, output_folder):
    cmd = [
        'python',
        'build.py',
        '--UE4',
        UE4,
        '--output',
        output_folder,
    ]
    return cmd

UE414 = [r'C:\Program Files\Epic Games\UE_4.14', r'C:/temp/unrealcv_built/414', 'log/UE414.log']
UE416 = [r'C:\Program Files\Epic Games\UE_4.16', r'C:/temp/unrealcv_built/416', 'log/UE416.log']
UE418 = [r'C:\Program Files\Epic Games\UE_4.18', r'C:/temp/unrealcv_built/418', 'log/UE418.log']
# Send the output to C:/temp to avoid cluttering current directory, which might cause failure to the build.

ue_versions = [UE414, UE416, UE418]

if not os.path.isdir('log'):
    os.mkdir('log')

for ue4 in ue_versions:
    print('-' * 80)
    print('Run configuration', ue4)
    log_filename = ue4[2]
    if os.path.isfile(log_filename):
        print('Skip because log exists')
        continue
    cmd = format_cmd(ue4[0], ue4[1])

    logfile = open(log_filename, 'wb')
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    for line in proc.stdout:
        sys.stdout.buffer.write(line)
        sys.stdout.flush()
        # see, https://stackoverflow.com/questions/908331/how-to-write-binary-data-in-stdout-in-python-3
        logfile.write(line)
    proc.wait()
    # with open(log_filename, 'w') as f:
    #     subprocess.call(cmd, stdout = f, stderr = f)

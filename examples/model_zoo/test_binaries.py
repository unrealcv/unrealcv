import subprocess, os

binaries = [
    'D:/unrealcv/Binaries/RealisticRendering/WindowsNoEditor/RealisticRendering.exe',
    'D:/unrealcv/Binaries/ArchinteriorsVol2Scene1/WindowsNoEditor/ArchinteriorsVol2Scene1.exe',
    'D:/unrealcv/Binaries/ArchinteriorsVol2Scene2/WindowsNoEditor/ArchinteriorsVol2Scene2.exe',
    'D:/unrealcv/Binaries/ArchinteriorsVol2Scene3/WindowsNoEditor/ArchinteriorsVol2Scene3.exe',
    'D:/unrealcv/Binaries/UrbanCity/WindowsNoEditor/UrbanCity.exe',
]

linux_binary_path = './UE4Binaries/{project_name}/LinuxNoEditor/{project_name}/Binaries/Linux/{project_name}'
project_names = [
    'RealisticRendering', 'ArchinteriorsVol2Scene1', 'ArchinteriorsVol2Scene2',
    'ArchinteriorsVol2Scene3', 'UrbanCity',
]

binaries += [linux_binary_path.format(project_name = v) for v in project_names]

if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.path.mkdir('output')

    for binary_path in binaries:
        project_name = os.path.basename(binary_path).split('.')[0]
        output_folder = os.path.join('output', project_name)
        if not os.path.isfile(binary_path):
            print('Can not find binary %s, skip' % binary_path)
            continue

        print('Testing %s ...' % binary_path)
        subprocess.call([
            'python', 'docs/reference/commands_demo.py',
            binary_path, '--output', output_folder
        ])

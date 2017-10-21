import subprocess, os

win_binary_path = 'UE4Binaries/{project_name}/WindowsNoEditor/{project_name}.exe'
linux_binary_path = './UE4Binaries/{project_name}/LinuxNoEditor/{project_name}/Binaries/Linux/{project_name}'
mac_binary_path = './UE4Binaries/{project_name}/MacNoEditor/{project_name}.app'

project_names = [
    'RealisticRendering', 'ArchinteriorsVol2Scene1', 'ArchinteriorsVol2Scene2',
    'ArchinteriorsVol2Scene3', 'UrbanCity', 'Matinee', 'PhotorealisticCharacter'
]

binaries = []
binaries += [linux_binary_path.format(project_name = v) for v in project_names]
binaries += [win_binary_path.format(project_name = v) for v in project_names]
binaries += [mac_binary_path.format(project_name = v) for v in project_names]

if __name__ == '__main__':
    if not os.path.isdir('output'):
        os.mkdir('output')

    for binary_path in binaries:
        project_name = os.path.basename(binary_path).split('.')[0]
        output_folder = os.path.join('output', project_name)
        if not os.path.isfile(binary_path) and not os.path.isdir(binary_path):
            print('Can not find binary "%s", skip' % binary_path)
            continue

        print('Testing %s ..., output will be saved to "%s"' % (binary_path, output_folder))
        subprocess.call([
            'python', 'examples/commands_demo.py',
            binary_path, '--output', output_folder
        ])

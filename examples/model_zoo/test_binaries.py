import subprocess

binaries = [
    dict(
        binary_path = 'D:/unrealcv/Binaries/ArchinteriorsVol2Scene1/WindowsNoEditor/ArchinteriorsVol2Scene1.exe',
        output_folder = 'output/arch1'
    ),
    dict(
        binary_path = 'D:/unrealcv/Binaries/ArchinteriorsVol2Scene2/WindowsNoEditor/ArchinteriorsVol2Scene2.exe',
        output_folder = 'output/arch2'
    ),
    dict(
        binary_path =
        'D:/unrealcv/Binaries/ArchinteriorsVol2Scene3/WindowsNoEditor/ArchinteriorsVol2Scene3.exe',
        output_folder = 'output/arch3'
    ),
    dict(
        binary_path = 'D:/unrealcv/Binaries/UrbanCity/WindowsNoEditor/UrbanCity.exe',
        output_folder = 'output/urbancity'
    ),
    dict(
        binary_path = 'D:/unrealcv/Binaries/RealisticRendering/WindowsNoEditor/RealisticRendering.exe',
        output_folder = 'output/rr'
    )
]

if __name__ == '__main__':
    for binary in binaries:
        subprocess.call(['python', 'docs/reference/commands_demo.py',
        binary['binary_path'], '--output', binary['output_folder']
        ])

import subprocess, os

ue4_win = r"C:\Program Files\Epic Games\UE_4.16"
ue4_linux = "/home/qiuwch/workspace/UE416"
ue4_mac = '/Users/Shared/Epic Games/UE_4.16'

win_uprojects = [
    r'C:\qiuwch\workspace\uprojects\UE4RealisticRendering\RealisticRendering.uproject',
    r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene1\ArchinteriorsVol2Scene1.uproject',
    r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene2\ArchinteriorsVol2Scene2.uproject',
    r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene3\ArchinteriorsVol2Scene3.uproject',
    r'C:\qiuwch\workspace\uprojects\UE4UrbanCity\UrbanCity.uproject',
    r'D:\workspace\uprojects\Matinee\Matinee.uproject',
    r'D:\workspace\uprojects\PhotorealisticCharacter\PhotorealisticCharacter2.uproject',
]

linux_uprojects = [
    os.path.expanduser('~/workspace/uprojects/UE4RealisticRendering/RealisticRendering.uproject'),
    os.path.expanduser('~/workspace/uprojects/UE4ArchinteriorsVol2Scene1/ArchinteriorsVol2Scene1.uproject'),
    os.path.expanduser('~/workspace/uprojects/UE4ArchinteriorsVol2Scene2/ArchinteriorsVol2Scene2.uproject'),
    os.path.expanduser('~/workspace/uprojects/UE4ArchinteriorsVol2Scene3/ArchinteriorsVol2Scene3.uproject'),
    os.path.expanduser("~/workspace/uprojects/UE4UrbanCity/UrbanCity.uproject"),
]

mac_uprojects = [
    os.path.expanduser('~/workspace/UnrealEngine/Templates/FP_FirstPerson/FP_FirstPerson.uproject'),
    os.path.expanduser('~/uprojects/RealisticRendering/RealisticRendering.uproject'),
    os.path.expanduser('~/uprojects/UE4ArchinteriorsVol2Scene1/ArchinteriorsVol2Scene1.uproject'),
    os.path.expanduser('~/uprojects/UE4ArchinteriorsVol2Scene2/ArchinteriorsVol2Scene2.uproject'),
    os.path.expanduser('~/uprojects/UE4ArchinteriorsVol2Scene3/ArchinteriorsVol2Scene3.uproject'),
    os.path.expanduser('~/uprojects/UE4UrbanCity/UrbanCity.uproject'),
]

uprojects = []

for uproject_path in win_uprojects:
    uproject_name = os.path.basename(uproject_path).split('.')[0]
    uprojects.append(
        dict(
            uproject_path = uproject_path,
            ue4_path = ue4_win,
            log_file = 'log/win_%s.log' % uproject_name
        ),
    )

for uproject_path in linux_uprojects:
    uproject_name = os.path.basename(uproject_path).split('.')[0]
    uprojects.append(
        dict(
            uproject_path = uproject_path,
            ue4_path = ue4_linux,
            log_file = 'log/linux_%s.log' % uproject_name
        ),
    )

for uproject_path in mac_uprojects:
    uproject_name = os.path.basename(uproject_path).split('.')[0]
    uprojects.append(
        dict(
            uproject_path = uproject_path,
            ue4_path = ue4_mac,
            log_file = 'log/mac_%s.log' % uproject_name
        ),
    )


if __name__ == '__main__':
    for uproject in uprojects:
        uproject_path = uproject['uproject_path']
        if not os.path.isfile(uproject_path):
            print("Can not find uproject file %s, skip this project" % uproject_path)
            continue

        cmd = [
                'python', 'build.py',
                '--UE4', uproject['ue4_path'],
                # '--output', uproject['output_folder'],
                uproject['uproject_path']
            ]
        print(cmd)
        subprocess.call(cmd,
            stdout = open(uproject['log_file'], 'w'))
        with open(uproject['log_file']) as f:
            lines = f.readlines()
            print(''.join(lines[-10:])) # Print the last few lines

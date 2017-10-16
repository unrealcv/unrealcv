import subprocess

ue4_win = r"C:\Program Files\Epic Games\UE_4.16"
ue4_linux = "/home/qiuwch/workspace/UE416"

uprojects = [
    dict(
        uproject_path = r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene1\ArchinteriorsVol2Scene1.uproject',
        ue4_path = ue4_win,
        log_file = 'log/win_scene1.log'
    ),
    dict(
        uproject_path = r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene2\ArchinteriorsVol2Scene2.uproject',
        ue4_path = ue4_win,
        log_file = 'log/win_scene2.log'
    ),
    dict(
        uproject_path = r'C:\qiuwch\workspace\uprojects\UE4ArchinteriorsVol2Scene3\ArchinteriorsVol2Scene3.uproject',
        ue4_path = ue4_win,
        log_file = 'log/win_scene3.log'
    ),
    dict(
        uproject_path = r'C:\qiuwch\workspace\uprojects\UE4RealisticRendering\RealisticRendering.uproject',
        ue4_path = ue4_win,
        log_file = 'log/win_rr.log'
    ),
    dict(
        uproject_path = r'C:\qiuwch\workspace\uprojects\UE4UrbanCity\UrbanCity.uproject',
        ue4_path = ue4_win,
        log_file = 'log/win_urbancity.log'
    ),
]

if __name__ == '__main__':
    for uproject in uprojects:
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

import os, sys

# Automatically package game content
def package_linux(conf, projectfile):
    cmd = '{UATScript} BuildCookRun -project={projectfile} -archivedirectory={outputfolder} -noP4 -platform=Linux -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'.format(
        projectfile = projectfile,
        UATScript = conf.UATScript,
        outputfolder = conf.outputfolder,
    )
    os.system(cmd)

def package_win(conf, projectfile):
    cmd = '"{UATScript}" BuildCookRun -project="{projectfile}" -archivedirectory={outputfolder} -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'.format(
        projectfile = projectfile,
        UATScript = conf.UATScript,
        outputfolder = conf.outputfolder,
    )
    # windows_cmd = 'cmd /C start "" "D:/Epic Games/4.11/Engine/Build/BatchFiles/RunUAT.bat" BuildCookRun -project=\"D:/Unreal Projects/FlyingCppDefault/FlyingCppDefault.uproject\" -archivedirectory=AutoPackaging -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive'
    # In windows, directly run shell command is a little tricky. Easier to save a bat file first.
    with open('package.bat', 'w') as f:
        f.write(cmd)

    windows_cmd = 'cmd /C start "" "package.bat"'
    os.system(windows_cmd)

platform = sys.platform
if platform == 'Win64' or platform == 'cygwin':
    package = package_win
if platform == 'Linux':
    package = package_linux

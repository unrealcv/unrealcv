import argparse
import os


class namedict:
    def __init__(self, **kargs):
        self.__dict__.update(kargs)



def getconf():
    import platform
    hostname = platform.node()
    conf = confs[hostname]
    return conf


confs = {
    'DESKTOP-00R4Q27':
    namedict(
        UATScript = 'D:/Epic Games/4.11/Engine/Build/BatchFiles/RunUAT.bat',
        outputfolder = 'D:/AutoPackaging',
        package = package_win,
        platform = 'Win64',
    ),
    'Linux':
    namedict(
        UATScript = '/home/qiuwch/workspace/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh',
        outputfolder = '/home/qiuwch/AutoPackaging',
        package = package_linux,
        platform = 'linux'
    ),
}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('projectfile') # Need to be converted to abs path
    parser.add_argument('--action', required=True, choices=['gen_project', 'package'])

    args = parser.parse_args()
    projectfile = os.path.abspath(args.projectfile).replace('/drives/d/', 'D:/').replace('/home/mobaxterm/d/', 'D:/')

    conf = getconf()
    if args.action == 'package':
        conf.package(conf, projectfile)

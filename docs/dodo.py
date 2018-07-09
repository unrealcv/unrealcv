def task_build_win():
    # cmd = 'cmd.exe /C dir'
    ue4_runuat = r'"C:\Program Files\Epic Games\UE_4.19\Engine\Build\BatchFiles\RunUAT.bat"'
    package_folder = r'"D:\qiuwch\workspace\unrealcv\Plugins\UnrealCV"'
    plugin_file = r'"D:\qiuwch\workspace\unrealcv\UnrealCV.uplugin"'
    cmd = r'cmd.exe /C {ue4_runuat} BuildPlugin -plugin={plugin_file} \
-package={package_folder} -rocket -targetplatforms=Win64 -compile'.format(**locals())
    return {
        'actions': [cmd],
        'verbosity': 2,
    }

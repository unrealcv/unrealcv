# Weichao Qiu @ 2018
# Utility commands to make unrealcv development easier

import os
ue4_root = '/home/qiuwch/UE419' # The configuration only for my machine
uat_script = os.path.join(ue4_root, '/Engine/Build/BatchFiles/RunUAT.sh')

def task_cleanup():
    cmd = 'rm -rf Binaries/ Intermediate/'
    return {
        'actions': [cmd],
        'verbosity': 2,
    }

def task_docs():
    # Use less code to achieve the same tasks
    sphinx = 'sphinx-build -n -b html ./docs/ ./docs/output/ -j 16'
    doxygen = 'doxygen ./docs/Doxyfile'
    
    preview = 'xdg-open ./docs/output/index.html'
    preview = 'xdg-open ./docs/doxygen/html/index.html'
    actions = [doxygen, sphinx]

    return {
        'actions': actions,
        'verbosity': 2,
    }

def task_build():
    # Build plugin
    abs_plugin_file = os.path.abspath('./UnrealCV.uplugin')
    # Avoid generating to current folder, otherwise the binary built by the editor might conflict
    abs_output_folder = os.path.abspath('/tmp/unrealcv_binary')
    cmd = os.path.join(uat_script, 'BuildPlugin -plugin={abs_plugin_file} \
    -package={abs_output_folder} -rocket -targetplatforms=Linux -compile'.format(**locals()))

    return {'actions': actions, 'verbosity': 2}

def task_package():
    # The RunUAT script needs to take in absolute path
    # Mainly used to check whether the package is successful
    abs_uproject_file = os.path.abspath('../CarAct.uproject')
    abs_output_folder = os.path.abspath('/tmp/CarAct_binary')
    cmd = os.path.join(uat_script, 'BuildCookRun -project={abs_uproject_file} \
    -archivedirectory={abs_output_folder} \
    -platform=Linux -clientconfig=Development \
    -noP4 -stage -pak -archive -cook -build'.format(**locals()))
    actions = [cmd]

    return {'actions': actions, 'verbosity': 2}
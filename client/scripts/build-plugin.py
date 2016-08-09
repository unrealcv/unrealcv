import os, sys
from ue4config import conf
import ue4util

UAT_script = conf['UATScript']
if not os.path.isfile(UAT_script):
    print('Can not find Automation Script of UE4 %s' % UAT_script)
    print('Please set UnrealEnginePath correctly first')
else:
    plugin_file = os.path.abspath('../../UnrealCV.uplugin')
    if sys.platform == 'cygwin':
        plugin_file = ue4util.cygwinpath_to_winpath(plugin_file)

    output_folder = os.path.abspath('./build')
    if sys.platform == 'cygwin':
        output_folder = ue4util.cygwinpath_to_winpath(output_folder)

    UAT_script = UAT_script.replace(' ', '\ ')
    cmd = '%s BuildPlugin -plugin=%s -package=%s -rocket' % (UAT_script, plugin_file, output_folder)
    print(cmd)
    os.system(cmd)

import os
UnrealEnginePath='~/workspace/UnrealEngine/Engine'

UATScript = os.path.join(UnrealEnginePath, 'Build/BatchFiles/RunUAT.sh')
if not os.path.isfile(UATScript):
    print('Can not find Automation Script of UE4 %s' % UATScript)
    print('Please set UnrealEnginePath correctly first')
FullPluginFile = os.path.abspath('UnrealCV.uplugin')
os.system('%s BuildPlugin -plugin=%s' % (UATScript, FullPluginFile))

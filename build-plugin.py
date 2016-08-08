import os
UnrealEnginePath='/home/qiuwch/workspace/UnrealEngine'
UATScript = os.path.join(UnrealEnginePath, 'Engine/Build/BatchFiles/RunUAT.sh')
FullPluginFile = os.path.abspath('UnrealCV.uplugin')
os.system('%s BuildPlugin -plugin=%s' % (UATScript, FullPluginFile))

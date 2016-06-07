import os
EnginePath = '/Users/Shared/UnrealEngine/4.12'
UATScript = os.path.join(EnginePath, 'Engine/Build/BatchFiles/RunUAT.command')

conf = {
    'EnginePath': EnginePath,
    'UATScript': UATScript,
    'OutputFolder': '~/AutoPackaging',
}

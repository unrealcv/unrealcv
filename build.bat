REM set the environment variable in console or in system variables UE4=D:\Epic Games\4.13\Engine\
"%UE4%/Build/BatchFiles/RunUAT.bat" BuildPlugin -plugin=%CD%/UnrealCV.uplugin -package=%CD%/built -rocket -targetplatforms=Win64

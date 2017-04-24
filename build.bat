set plugin_file=%CD%/UnrealCV.uplugin
set package_folder=%CD%/Plugins/UnrealCV
set UE4=C:\Program Files\Epic Games\UE_4.14
REM TODO: Check the existence of UE4 folder and give a useful hint for user
REM Modify the UE4 variable to point to your UE4 installation path
"%UE4%/Engine/Build/BatchFiles/RunUAT.bat" BuildPlugin -plugin=%plugin_file% -package=%package_folder% -rocket -targetplatforms=Win64
xcopy /E /Y Plugins\UnrealCV\Binaries %CD%

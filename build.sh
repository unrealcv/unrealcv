#!/bin/bash
echo "Set the environment variable UE4 in the console or in the system variables"
echo "For example in my Linux machine, I use 'export UE4=/home/qiuwch/UnrealEngine/4.13'"
plugin_file=`pwd`/UnrealCV.uplugin
package_folder=`pwd`/Plugins/UnrealCV
"${UE4}"/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin -plugin=${plugin_file} -package=${package_folder} -rocket -targetplatforms=Linux+Mac

#!/bin/bash
echo "Set the environment variable UE4 in the console or in the system variables"
echo "For example in my Linux machine, I use 'export UE4=/home/qiuwch/UnrealEngine/4.13'"
${UE4}/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin -plugin=`pwd`/UnrealCV.uplugin -package=`pwd`/built -rocket -targetplatforms=Linux+Mac

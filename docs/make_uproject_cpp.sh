# Convert a blueprint only project to a cpp project by 
# adding some empty cpp code to fool the checking code
uproject=`ls *.uproject`

# Make sure I am in a uproject folder
echo ${uproject}
if [ ! -z ${uproject} ]; then
    echo Make a folder called Source
    mkdir -p Source
fi

build_rule="using UnrealBuildTool; public class dummyTarget : TargetRules { public dummyTarget(TargetInfo Target) { } }"
echo ${build_rule} > Source/dummy.Target.cs

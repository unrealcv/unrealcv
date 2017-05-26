# Convert a blueprint only project to a cpp project by
# adding some empty cpp code to fool the checking code
uproject=`ls *.uproject`

# Make sure I am in a uproject folder
echo ${uproject}
if [ -z ${uproject} ]; then
    exit
fi

echo Make a folder called Source
mkdir -p Source/playground

game_target='using UnrealBuildTool; public class playgroundTarget : TargetRules { public playgroundTarget(TargetInfo Target) { Type = TargetType.Game; }}'
echo ${game_target} > Source/playground.Target.cs

editor_target='using UnrealBuildTool; public class playgroundEditorTarget : TargetRules { public playgroundEditorTarget(TargetInfo Target) { Type = TargetType.Editor; }}'
echo ${editor_target} > Source/playgroundEditor.Target.cs

build_rule='using UnrealBuildTool; public class playground : ModuleRules { public playground(TargetInfo Target) { PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore"}); }}'
echo ${build_rule} > Source/playground/playground.Build.cs

playground_entry_code='#include "EngineMinimal.h"
#include "playground.h"
IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, playground, "playground" );'
echo ${playground_entry_code} > Source/playground/playground.cpp

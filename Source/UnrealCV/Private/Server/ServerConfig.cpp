// Weichao Qiu @ 2016
#include "ServerConfig.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/ConfigCacheIni.h"
#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistryModule.h"


#include "UnrealcvLog.h"

// TODO: Try to simplify the implementation of this class
FServerConfig::FServerConfig()
{
	// Default configuration
	// ConfigFile = GGameUserSettingsIni;
	ConfigFile = "unrealcv.ini";
	CoreSection = "UnrealCV.Core";

	// Default value, will be unchanged if the config is missing.
	Port = 9000; 
	Width = 640;
	Height = 480;
	FOV = 90.0f;
	EnableInput = true;
	ExitOnFailure = false;
	EnableRightEye = false;

	SupportedModes.Add(TEXT("lit"));
	SupportedModes.Add(TEXT("depth"));
//	SupportedModes.Add(TEXT("vis_depth"));
//	SupportedModes.Add(TEXT("plane_depth"));
	// SupportedModes.Add(TEXT("debug"));
	SupportedModes.Add(TEXT("object_mask"));
	SupportedModes.Add(TEXT("normal"));
	// SupportedModes.Add(TEXT("wireframe"));
//	SupportedModes.Add(TEXT("default"));

	this->Load(); 
	this->Save(); // Flush the default config to the disk if file not exist.
	this->ParseCmdArgs();

	UE_LOG(LogUnrealCV, Warning, TEXT("Port: %d"), this->Port);
	UE_LOG(LogUnrealCV, Warning, TEXT("Width: %d"), this->Width);
	UE_LOG(LogUnrealCV, Warning, TEXT("Height: %d"), this->Height);
	UE_LOG(LogUnrealCV, Warning, TEXT("FOV: %f"), this->FOV);
	UE_LOG(LogUnrealCV, Warning, TEXT("EnableInput: %s"), *BoolToString(this->EnableInput));
	UE_LOG(LogUnrealCV, Warning, TEXT("EnableRightEye: %s"), *BoolToString(this->EnableRightEye));
}

void FServerConfig::ParseCmdArgs()
{
	// https://answers.unrealengine.com/questions/123559/can-we-make-command-line-arguments.html
	// Use command line argument to overwrite config file.
	int ArgPort;
	// Use cvport to avoid conflict with UE4 default
	if (FParse::Value(FCommandLine::Get(), TEXT("cvport"), ArgPort)) 
	{
		Port = ArgPort;
	}

	FString LsFolder;
	if (FParse::Value(FCommandLine::Get(), TEXT("cvls"), LsFolder)) 
	{
		ListAsset(LsFolder);
		FGenericPlatformMisc::RequestExit(false);
	}
}

void FServerConfig::ListAsset(FString LsFolder)
{
	// https://answers.unrealengine.com/questions/145345/how-to-get-a-list-of-assets-from-code.html
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Need to do this if running in the editor with -game to make sure that the assets in the following path are available
	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/Game/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan);

	TArray<FAssetData> MeshAssetList;
	bool bRecursive = true;
	AssetRegistry.GetAssetsByPath(FName(*LsFolder), MeshAssetList, bRecursive);

	for (FAssetData& AssetData : MeshAssetList)
	{
		UE_LOG(LogUnrealCV, Display, TEXT("%s"), *AssetData.GetFullName());
	}
}

FString FServerConfig::BoolToString(bool Value) const
{
	if (Value)
	{
		return FString::Printf(TEXT("true"));
	}
	else
	{
		return FString::Printf(TEXT("false"));
	}
}

FString FServerConfig::ToString() {
	FString Msg = "";
	FString AbsConfigFile = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ConfigFile);
	Msg += FString::Printf(TEXT("Config file: %s\n"), *AbsConfigFile);
	Msg += FString::Printf(TEXT("Port: %d\n"), this->Port);
	Msg += FString::Printf(TEXT("Width: %d\n"), this->Width);
	Msg += FString::Printf(TEXT("Height: %d\n"), this->Height);
	Msg += FString::Printf(TEXT("FOV: %f\n"), this->FOV);
	Msg += FString::Printf(TEXT("EnableInput: %s\n"), *BoolToString(this->EnableInput));
	Msg += FString::Printf(TEXT("EnableRightEye: %s\n"), *BoolToString(this->EnableRightEye));
	return Msg;
}

bool FServerConfig::Load() {
	if (!GConfig) return false;

	UE_LOG(LogUnrealCV, Warning, TEXT("Loading config"));

	// Assume the value will not be overwrote if the read failed
	GConfig->GetInt(*CoreSection, TEXT("Port"), this->Port, this->ConfigFile);
	GConfig->GetInt(*CoreSection, TEXT("Width"), this->Width, this->ConfigFile);
	GConfig->GetInt(*CoreSection, TEXT("Height"), this->Height, this->ConfigFile);
	GConfig->GetFloat(*CoreSection, TEXT("FOV"), this->FOV, this->ConfigFile);
	GConfig->GetBool(*CoreSection, TEXT("EnableInput"), this->EnableInput, this->ConfigFile);
	GConfig->GetBool(*CoreSection, TEXT("EnableRightEye"), this->EnableRightEye, this->ConfigFile);


	return true;
}

bool FServerConfig::Save()
{
	// Reference: https://wiki.unrealengine.com/Config_Files,_Read_%26_Write_to_Config_Files
	if (!GConfig) return false;

	GConfig->SetInt(*CoreSection, TEXT("Port"), this->Port, this->ConfigFile);
	GConfig->SetInt(*CoreSection, TEXT("Width"), this->Width, this->ConfigFile);
	GConfig->SetInt(*CoreSection, TEXT("Height"), this->Height, this->ConfigFile);
	GConfig->SetFloat(*CoreSection, TEXT("FOV"), this->FOV, this->ConfigFile);
	GConfig->SetBool(*CoreSection, TEXT("EnableInput"), this->EnableInput, this->ConfigFile);
	GConfig->SetBool(*CoreSection, TEXT("EnableRightEye"), this->EnableRightEye, this->ConfigFile);

	bool Read = false;
	GConfig->Flush(Read, this->ConfigFile);
	return true;
}

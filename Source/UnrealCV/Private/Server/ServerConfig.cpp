// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "ServerConfig.h"
#include "Runtime/Core/Public/HAL/FileManager.h"
#include "Runtime/Core/Public/Misc/ConfigCacheIni.h"
#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Modules/ModuleManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/AssetData.h"
#include "UnrealcvLog.h"

FServerConfig::FServerConfig()
{
	ConfigFile  = TEXT("unrealcv.ini");
	CoreSection = TEXT("UnrealCV.Core");

	SupportedModes.Add(TEXT("lit"));
	SupportedModes.Add(TEXT("depth"));
	SupportedModes.Add(TEXT("object_mask"));
	SupportedModes.Add(TEXT("normal"));

	Load();
	Save(); // Flush defaults to disk if the file does not exist yet.
	ParseCmdArgs();

	UE_LOG(LogUnrealCV, Display, TEXT("Config  Port=%d  Size=%dx%d  FOV=%.1f  Input=%s  RightEye=%s"),
		Port, Width, Height, FOV,
		bEnableInput    ? TEXT("on") : TEXT("off"),
		bEnableRightEye ? TEXT("on") : TEXT("off"));
}

void FServerConfig::ParseCmdArgs()
{
	int32 ArgPort = 0;
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

void FServerConfig::ListAsset(const FString& Folder)
{
	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FString> PathsToScan;
	PathsToScan.Add(TEXT("/Game/"));
	AssetRegistry.ScanPathsSynchronous(PathsToScan);

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssetsByPath(FName(*Folder), Assets, /*bRecursive=*/true);

	for (const FAssetData& Asset : Assets)
	{
		UE_LOG(LogUnrealCV, Display, TEXT("%s"), *Asset.GetFullName());
	}
}

FString FServerConfig::ToString() const
{
	const FString AbsPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ConfigFile);
	return FString::Printf(
		TEXT("Config file: %s\nPort: %d\nWidth: %d\nHeight: %d\nFOV: %.1f\n"
		     "EnableInput: %s\nEnableRightEye: %s"),
		*AbsPath, Port, Width, Height, FOV,
		bEnableInput    ? TEXT("true") : TEXT("false"),
		bEnableRightEye ? TEXT("true") : TEXT("false"));
}

bool FServerConfig::Load()
{
	if (!GConfig) { return false; }

	UE_LOG(LogUnrealCV, Display, TEXT("Loading configuration from %s"), *ConfigFile);

	GConfig->GetInt   (*CoreSection, TEXT("Port"),           Port,            ConfigFile);
	GConfig->GetInt   (*CoreSection, TEXT("Width"),          Width,           ConfigFile);
	GConfig->GetInt   (*CoreSection, TEXT("Height"),         Height,          ConfigFile);
	GConfig->GetFloat (*CoreSection, TEXT("FOV"),            FOV,             ConfigFile);
	GConfig->GetBool  (*CoreSection, TEXT("EnableInput"),    bEnableInput,    ConfigFile);
	GConfig->GetBool  (*CoreSection, TEXT("EnableRightEye"), bEnableRightEye, ConfigFile);

	return true;
}

bool FServerConfig::Save()
{
	if (!GConfig) { return false; }

	GConfig->SetInt   (*CoreSection, TEXT("Port"),           Port,            ConfigFile);
	GConfig->SetInt   (*CoreSection, TEXT("Width"),          Width,           ConfigFile);
	GConfig->SetInt   (*CoreSection, TEXT("Height"),         Height,          ConfigFile);
	GConfig->SetFloat (*CoreSection, TEXT("FOV"),            FOV,             ConfigFile);
	GConfig->SetBool  (*CoreSection, TEXT("EnableInput"),    bEnableInput,    ConfigFile);
	GConfig->SetBool  (*CoreSection, TEXT("EnableRightEye"), bEnableRightEye, ConfigFile);

	GConfig->Flush(false, ConfigFile);
	return true;
}

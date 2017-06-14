#include "UnrealCVPrivate.h"
#include "ServerConfig.h"

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

	this->Load(); 
	this->Save(); // Flush the default config to the disk if file not exist.
}

FString FServerConfig::ToString()
{
	FString Msg = "";
	FString AbsConfigFile = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*ConfigFile); 
	Msg += FString::Printf(TEXT("Config file: %s\n"), *AbsConfigFile);
	Msg += FString::Printf(TEXT("Port: %d\n"), this->Port);
	Msg += FString::Printf(TEXT("Width: %d\n"), this->Width);
	Msg += FString::Printf(TEXT("Height: %d\n"), this->Height);
	return Msg;
}

bool FServerConfig::Load()
{
	if (!GConfig) return false;

	// Assume the value will not be overwrote if the read failed
	GConfig->GetInt(*CoreSection, TEXT("Port"), this->Port, this->ConfigFile);
	GConfig->GetInt(*CoreSection, TEXT("Width"), this->Width, this->ConfigFile);
	GConfig->GetInt(*CoreSection, TEXT("Height"), this->Height, this->ConfigFile);

	return true;
}

bool FServerConfig::Save()
{
	// Reference: https://wiki.unrealengine.com/Config_Files,_Read_%26_Write_to_Config_Files
	if (!GConfig) return false;

	GConfig->SetInt(*CoreSection, TEXT("Port"), this->Port, this->ConfigFile);
	GConfig->SetInt(*CoreSection, TEXT("Width"), this->Width, this->ConfigFile);
	GConfig->SetInt(*CoreSection, TEXT("Height"), this->Height, this->ConfigFile);

	bool Read = false;
	GConfig->Flush(Read, this->ConfigFile);
	return true;
}

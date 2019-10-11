// Weichao Qiu @ 2016
#pragma once

#include "CoreMinimal.h"

class FServerConfig
{
private:
	FString BoolToString(bool Value) const;

	FString ConfigFile;
	FString CoreSection;
public:
	int Port;
	int Width;
	int Height;
	float FOV;
	bool EnableInput;
	bool ExitOnFailure;
	bool EnableRightEye;

	TArray<FString> SupportedModes;

	FServerConfig();

	/** Serialize this configuration to a string for debugging */
	FString ToString();

	/** Save FServerConfig to file */
	bool Save();

	/** Load FServerConfig from configuration file */
	bool Load();

	// Load config from command line argument
	void ParseCmdArgs();

	void ListAsset(FString LsFolder);
};

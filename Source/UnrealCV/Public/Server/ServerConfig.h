// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

/**
 * Runtime configuration for the UnrealCV server.
 *
 * Values are loaded from an INI file (unrealcv.ini), can be overridden via
 * command-line flags (-cvport, -cvls), and are flushed back to disk so that
 * the file always reflects the active defaults.
 */
class FServerConfig
{
public:
	int32 Port            = 9000;
	int32 Width           = 640;
	int32 Height          = 480;
	float FOV             = 90.0f;
	bool  bEnableInput    = true;
	bool  bExitOnFailure  = false;
	bool  bEnableRightEye = false;

	TArray<FString> SupportedModes;

	FServerConfig();

	/** Human-readable dump of all settings. */
	FString ToString() const;

	/** Persist current values to the config file. */
	bool Save();

	/** Reload values from the config file (missing keys keep defaults). */
	bool Load();

	/** Override settings from command-line arguments. */
	void ParseCmdArgs();

	/** List assets under a folder and request exit (diagnostic tool). */
	static void ListAsset(const FString& Folder);

private:
	FString ConfigFile;
	FString CoreSection;
};

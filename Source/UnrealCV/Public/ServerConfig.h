#pragma once
class FServerConfig
{
private:
	FString ConfigFile;
	FString CoreSection;
public:
	int Port;
	int Width;
	int Height;
	float FOV;
	bool EnableInput;
	bool ExitOnFailure;

	FServerConfig();

	/** Serialize this configuration to a string for debugging */
	FString ToString();

	/** Save FServerConfig to file */
	bool Save();

	/** Load FServerConfig from configuration file */
	bool Load();
};

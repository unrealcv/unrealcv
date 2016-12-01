#pragma once
class FServerConfig
{
private:
	FString ConfigFile;
public:
	int Port;
	int Width;
	int Height;

	FServerConfig() : ConfigFile(GGameUserSettingsIni), Port(9000), Width(640), Height(480)
	{
	}

	/** Save FServerConfig to file */
	bool Save();

	/** Load FServerConfig from configuration file */
	bool Load();
};
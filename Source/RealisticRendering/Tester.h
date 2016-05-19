#pragma once

#include "RealisticRendering.h"
#include "NetworkManager.h"
#include "UE4CVServer.h"
#include "CommandDispatcher.h"


class ITester
{
public:
	virtual void Run() = 0;
	virtual void Init() {};
};

class NetworkManagerTester : public ITester
{
private:
	UNetworkManager* NetworkManager;

public:
	NetworkManagerTester();
	// Run test once
	void Run();
	// Set up the environment
	void Init();

	void LogMessage(const FString& Message);
};


class UE4CVServerTester : public ITester
{
private:
	FUE4CVServer* Server;
	FCommandDispatcher* CommandDispatcher;

public: 
	UE4CVServerTester(FCommandDispatcher* InCommandDispatcher);
	void Run();
	void Init();
};

class FilePathTester : public ITester
{
public: 
	void Run();
};
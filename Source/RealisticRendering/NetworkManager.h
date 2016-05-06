// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Networking.h"
#include "NetworkManager.generated.h"

/**
 *
 */
DECLARE_EVENT_OneParam(UNetworkManager, FReceivedEvent, FString) // TODO: Consider add a pointer to NetworkManager itself
UCLASS()
class REALISTICRENDERING_API UNetworkManager : public UObject
{
	GENERATED_BODY()

private:
	FSocket* Listener; // TODO: Replace with intelligent pointer here.
	FSocket* ConnectionSocket;

	FTimerManager* WorldTimerManager;
	FReceivedEvent ReceivedEvent;
	void BroadcastReceived(const FString& Message)
	{
		ReceivedEvent.Broadcast(Message); // TODO: A call to return message is required by design.
	}
public:
	FReceivedEvent& OnReceived() { return ReceivedEvent; }

	UPROPERTY()
	uint32 PortNum = 9000;
	UPROPERTY()
	bool bIsConnected;
	UWorld* World;

	UNetworkManager();
	bool IsConnected();
	void SendMessage(FString Message);
	void ListenSocket();
	void WaitConnection();
	void WaitData();
	FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);
	~UNetworkManager();

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Networking.h"
#include "NetworkManager.generated.h"

/**
 *
 */
DECLARE_EVENT_OneParam(UNetworkManager, FReceivedEvent, FString)
// TODO: Consider add a pointer to NetworkManager itself
// TODO: Add connected event
UCLASS()
class REALISTICRENDERING_API UNetworkManager : public UObject 
// NetworkManager needs to be an UObject, because TimerManager can only support method of an UObject
{
	GENERATED_BODY()

private:
	FSocket* Listener; // TODO: Replace with intelligent pointer here.
	FSocket* ConnectionSocket;

	FReceivedEvent ReceivedEvent;
	
	void Expired();
	void HandleRawMessage(FString RawMessage);

	void BroadcastReceived(const FString& Message)
	{
		ReceivedEvent.Broadcast(Message); // TODO: A call to return message is required by design.
	}
	void ResetTimeoutTimer();
public:
	FReceivedEvent& OnReceived() { return ReceivedEvent; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PortNum = 9000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsConnected = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ListenIP = "0.0.0.0"; // TODO: this is hard coded right now
	FString HeartBeatMessage = "echo";

	UWorld* World;

	FTimerHandle CheckConnectionTimerHandle;
	uint32 ConnectionTimeout = 3;

	UNetworkManager();
	bool IsConnected();
	void SendMessage(FString Message);
	void ListenSocket();
	void WaitConnection();
	void WaitData();
	FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);
	~UNetworkManager();

};

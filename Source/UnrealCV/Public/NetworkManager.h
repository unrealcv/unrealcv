// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Networking.h"
#include "NetworkMessage.h"
#include "NetworkManager.generated.h"

/* 
* a simplified version from FNFSMessageHeader, without CRC check 
*/
class FSocketMessageHeader
{
	/* Error checking */
	uint32 Magic = 0;

	/* Payload Size */
	uint32 PayloadSize = 0;

	static uint32 DefaultMagic;
public:
	FSocketMessageHeader(const TArray<uint8>& Payload)
	{
		PayloadSize = Payload.Num();  // What if PayloadSize is 0
		Magic = FSocketMessageHeader::DefaultMagic;
	}

	static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);
};

DECLARE_EVENT_OneParam(UNetworkManager, FReceivedEvent, const FString&)
// TODO: Consider add a pointer to NetworkManager itself
// TODO: Add connected event


/**
 * Server to send and receive message
 */
UCLASS()
class UNetworkManager : public UObject
// NetworkManager needs to be an UObject, because TimerManager can only support method of an UObject
{
	GENERATED_BODY()

public:
	// TODO: Make these property effective
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 PortNum = 9000;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsConnected = false;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FString ListenIP = "0.0.0.0"; // TODO: this is hard coded right now

	void Start();
	bool SendMessage(const FString& Message);

	FReceivedEvent& OnReceived() { return ReceivedEvent;  } // The reference can not be changed

private:
	bool Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	FSocket* ConnectionSocket = NULL; // FSimpleAbstractSocket's receive is hard to use for non-blocking mode
	FTcpListener* TcpListener;
	~UNetworkManager();

	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/* Event handler */
	FReceivedEvent ReceivedEvent;
	void BroadcastReceived(const FString& Message)
	{
		ReceivedEvent.Broadcast(Message);
	}
};

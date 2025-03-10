#pragma once

#include "CoreMinimal.h"
#include "Runtime/Networking/Public/Common/TcpListener.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h"
#include "Runtime/Core/Public/Serialization/ArrayReader.h"
#include "UnrealcvLog.h"
#include "HAL/Runnable.h"
#include "SocketSubsystem.h"

class FMultiSocketMessageHeader
{
	/** Error checking */
	uint32 Magic = 0;

	/** Payload Size */
	uint32 PayloadSize = 0;

	static uint32 DefaultMagic;
public:
	FMultiSocketMessageHeader(const TArray<uint8>& Payload)
	{
		PayloadSize = Payload.Num();  // What if PayloadSize is 0
		Magic = FMultiSocketMessageHeader::DefaultMagic;
	}

	/** Receive packages and strip header */
	static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);
};

DECLARE_EVENT_TwoParams(FClientHandler, FMultiReceivedEvent, const FString&, const FString&);
DECLARE_EVENT_OneParam(FClientHandler, FMultiConnectedEvent, const FString&);

/**
 * Handles communication with a single client in a separate thread.
 */
class FClientHandler : public FRunnable
{
public:
    FClientHandler(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
    virtual ~FClientHandler();

	//Name of the thread
	FString ThreadName;

	//Thread to create
	FRunnableThread* Thread;

protected:



    //Determine if the stop function is called to stop this thread
    bool bStop;

    //Sockect this thread is using.
    FSocket* ConnectionSocket;

	/** Event handler for event `Received` */
	FMultiReceivedEvent ReceivedEvent;

	/** Event handler for event `Connected` */
	FMultiConnectedEvent ConnectedEvent;


	/** Broadcast event `Received` */
	void BroadcastReceived(const FString& Endpoint, const FString& Message)
	{
		ReceivedEvent.Broadcast(Endpoint, Message);
	}

	/** Broadcast event `Connected` */
	void BroadcastConnected(const FString& Message)
	{
		ConnectedEvent.Broadcast(Message);
	}

public:

	FMultiReceivedEvent& OnReceived() { return ReceivedEvent; } // The reference can not be changed
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

};

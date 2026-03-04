// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Common/TcpListener.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h"
#include "Runtime/Core/Public/Serialization/ArrayReader.h"

#include "TcpServer.generated.h"

/**
 * Lightweight message framing header for socket communication.
 * Wire format: [uint32 Magic][uint32 PayloadSize][...Payload bytes...]
 */
class FSocketMessageHeader
{
public:
	explicit FSocketMessageHeader(const TArray<uint8>& Payload)
		: Magic(DefaultMagic)
		, PayloadSize(static_cast<uint32>(Payload.Num()))
	{
	}

	static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);

	static uint32 DefaultMagic;

private:
	uint32 Magic = 0;
	uint32 PayloadSize = 0;
};

DECLARE_EVENT_TwoParams(UTcpServer, FTcpReceivedEvent,  const FString&, const FString&);
DECLARE_EVENT_OneParam (UTcpServer, FTcpErrorEvent,     const FString&);
DECLARE_EVENT_OneParam (UTcpServer, FTcpConnectedEvent, const FString&);

/**
 * Single-client TCP message server.
 */
UCLASS()
class UNREALCV_API UTcpServer : public UObject
{
	GENERATED_BODY()

public:
	int32 PortNum = 0;

	bool IsConnected() const;
	bool IsListening() const { return bIsListening; }

	bool Start(int32 InPortNum);
	bool SendMessage(const FString& Message);
	bool SendData(const TArray<uint8>& Payload);

	FTcpReceivedEvent&  OnReceived()  { return ReceivedEvent; }
	FTcpErrorEvent&     OnError()     { return ErrorEvent; }

private:
	bool bIsListening = false;

	bool Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	FSocket* ConnectionSocket = nullptr;
	TSharedPtr<FTcpListener> TcpListener;

	virtual ~UTcpServer() override;

	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	FTcpReceivedEvent  ReceivedEvent;
	FTcpErrorEvent     ErrorEvent;
	FTcpConnectedEvent ConnectedEvent;

	void BroadcastReceived(const FString& Endpoint, const FString& Message)
	{
		ReceivedEvent.Broadcast(Endpoint, Message);
	}
	void BroadcastConnected(const FString& Message)
	{
		ConnectedEvent.Broadcast(Message);
	}
};

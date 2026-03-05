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
class UNREALCV_API FSocketMessageHeader
{
public:
	explicit FSocketMessageHeader(const TArray<uint8>& Payload)
		: Magic(DefaultMagic)
		, PayloadSize(static_cast<uint32>(Payload.Num()))
	{
	}

	[[nodiscard]] static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	[[nodiscard]] static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);

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
	[[nodiscard]] int32 GetPortNum() const { return PortNum; }

	[[nodiscard]] bool IsConnected() const;
	[[nodiscard]] bool IsListening() const { return bIsListening; }

	[[nodiscard]] bool Start(int32 InPortNum);
	[[nodiscard]] bool SendMessage(const FString& Message);
	[[nodiscard]] bool SendData(const TArray<uint8>& Payload);

	FTcpReceivedEvent&  OnReceived()  { return ReceivedEvent; }
	FTcpErrorEvent&     OnError()     { return ErrorEvent; }

private:
	int32 PortNum = 0;
	bool bIsListening = false;

	bool OnClientConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	FSocket* ConnectionSocket = nullptr;
	TSharedPtr<FTcpListener> TcpListener;

	virtual ~UTcpServer() override;

	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	void CleanupConnection();

	FTcpReceivedEvent  ReceivedEvent;
	FTcpErrorEvent     ErrorEvent;
	FTcpConnectedEvent ConnectedEvent;
};

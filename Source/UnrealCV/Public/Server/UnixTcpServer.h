// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
// Original TCP: Weichao Qiu, 2016. UDS extension: Hai Ci, 2022.
#pragma once

#include "Runtime/Sockets/Public/Sockets.h"
#include "Runtime/Networking/Public/Common/TcpListener.h"
#include "Runtime/Networking/Public/Interfaces/IPv4/IPv4Endpoint.h"
#include "Runtime/Core/Public/Serialization/ArrayReader.h"

#if PLATFORM_LINUX
#include <cstdlib>
#include <cstdio>
#include <cstddef>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <unistd.h>
#include <cctype>
#endif

#include "UnixTcpServer.generated.h"

/**
 * Message framing header with TCP and UDS transport helpers.
 */
class FUnixSocketMessageHeader
{
public:
	explicit FUnixSocketMessageHeader(const TArray<uint8>& Payload)
		: Magic(DefaultMagic)
		, PayloadSize(static_cast<uint32>(Payload.Num()))
	{
	}

	static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);
	static bool WrapAndSendPayloadUDS(const TArray<uint8>& Payload, int Fd);
	static bool ReceivePayloadUDS(FArrayReader& OutPayload, int Fd);

	static uint32 DefaultMagic;

private:
	uint32 Magic = 0;
	uint32 PayloadSize = 0;
};

DECLARE_EVENT_TwoParams(UUnixTcpServer, FReceivedEvent, const FString&, const FString&);
DECLARE_EVENT_OneParam (UUnixTcpServer, FErrorEvent,    const FString&);
DECLARE_EVENT_OneParam (UUnixTcpServer, FConnectedEvent,const FString&);

/**
 * Single-client server supporting both Internet TCP and Linux UDS transports.
 */
UCLASS()
class UNREALCV_API UUnixTcpServer : public UObject
{
	GENERATED_BODY()

public:
	int32 PortNum = -1;

	bool IsConnected() const;
	bool IsListening() const { return bIsListening; }

	bool Start(int32 InPortNum);

	bool SendMessage(const FString& Message);
	bool SendData(const TArray<uint8>& Payload);
	bool SendMessageINet(const FString& Message);
	bool SendDataINet(const TArray<uint8>& Payload);
	bool SendMessageUDS(const FString& Message);
	bool SendDataUDS(const TArray<uint8>& Payload);

	FReceivedEvent&  OnReceived()  { return ReceivedEvent; }
	FErrorEvent&     OnError()     { return ErrorEvent; }

private:
	bool bIsListening = false;
	bool bIsUDS       = false;

	bool Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	FSocket* ConnectionSocket = nullptr;
	int UDS_ConnFd   = -1;
	int UDS_ListenFd = -1;

	TSharedPtr<FTcpListener> TcpListener;

	virtual ~UUnixTcpServer() override;

	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageServiceINet(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageServiceUDS();

	FReceivedEvent  ReceivedEvent;
	FErrorEvent     ErrorEvent;
	FConnectedEvent ConnectedEvent;

	void BroadcastReceived(const FString& Endpoint, const FString& Message)
	{
		ReceivedEvent.Broadcast(Endpoint, Message);
	}
	void BroadcastConnected(const FString& Message)
	{
		ConnectedEvent.Broadcast(Message);
	}
};

// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
// Original TCP: Weichao Qiu, 2016. UDS extension: Hai Ci, 2022.
#pragma once

#include "Sockets.h"
#include "Common/TcpListener.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Serialization/ArrayReader.h"
#include "Templates/Atomic.h"

#include "UnixTcpServer.generated.h"

/**
 * Message framing header with TCP and UDS transport helpers.
 */
class UNREALCV_API FUnixSocketMessageHeader
{
public:
	explicit FUnixSocketMessageHeader(const TArray<uint8>& Payload)
		: Magic(DefaultMagic)
		, PayloadSize(static_cast<uint32>(Payload.Num()))
	{
	}

	[[nodiscard]] static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	[[nodiscard]] static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);
	[[nodiscard]] static bool WrapAndSendPayloadUDS(const TArray<uint8>& Payload, int Fd);
	[[nodiscard]] static bool ReceivePayloadUDS(FArrayReader& OutPayload, int Fd);

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
 *
 * On Linux, after the first TCP client disconnects the server transitions
 * to a Unix Domain Socket listener (for lower-latency local communication).
 */
UCLASS()
class UNREALCV_API UUnixTcpServer : public UObject
{
	GENERATED_BODY()

public:
	[[nodiscard]] int32 GetPortNum() const { return PortNum; }

	[[nodiscard]] bool IsConnected() const;
	[[nodiscard]] bool IsListening() const { return bIsListening; }

	[[nodiscard]] bool Start(int32 InPortNum);

	[[nodiscard]] bool SendMessage(const FString& Message);
	[[nodiscard]] bool SendData(const TArray<uint8>& Payload);
	[[nodiscard]] bool SendMessageINet(const FString& Message);
	[[nodiscard]] bool SendDataINet(const TArray<uint8>& Payload);
	[[nodiscard]] bool SendMessageUDS(const FString& Message);
	[[nodiscard]] bool SendDataUDS(const TArray<uint8>& Payload);

	FReceivedEvent&  OnReceived()  { return ReceivedEvent; }
	FErrorEvent&     OnError()     { return ErrorEvent; }

private:
	int32 PortNum     = -1;
	bool bIsListening = false;
	TAtomic<bool> bIsUDS { false };

	bool OnClientConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	FSocket* ConnectionSocket = nullptr;

#if PLATFORM_LINUX
	int UDS_ConnFd   = -1;
	int UDS_ListenFd = -1;
#endif

	TSharedPtr<FTcpListener> TcpListener;

	virtual ~UUnixTcpServer() override;

	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageServiceINet(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);
	bool StartMessageServiceUDS();

	void CleanupConnection();

	FReceivedEvent  ReceivedEvent;
	FErrorEvent     ErrorEvent;
	FConnectedEvent ConnectedEvent;
};

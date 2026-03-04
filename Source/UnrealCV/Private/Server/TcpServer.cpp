// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "TcpServer.h"
#include "SocketUtils.h"
#include "Runtime/Core/Public/Serialization/BufferArchive.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"
#include "HAL/PlatformProcess.h"
#include "UnrealcvLog.h"
#include "UnrealcvShim.h"

uint32 FSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

namespace
{
constexpr uint32 MaxPayloadSizeBytes = 256u * 1024u * 1024u;
}

// ---------------------------------------------------------------------------
// FSocketMessageHeader
// ---------------------------------------------------------------------------

bool FSocketMessageHeader::WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FSocketMessageHeader Header(Payload);

	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 TotalSent   = 0;
	int32 Remaining   = Ar.Num();
	int32 RetriesLeft = 100;

	while (Remaining > 0)
	{
		int32 AmountSent = 0;
		Socket->Send(Ar.GetData() + TotalSent, Ar.Num() - TotalSent, AmountSent);

		if (AmountSent == -1)
		{
			if (--RetriesLeft < 0)
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Send failed after retries. Expected %d, sent %d."), Ar.Num(), TotalSent);
				return false;
			}
			FPlatformProcess::Sleep(0.001f);
			continue;
		}

		UE_LOG(LogUnrealCV, Verbose, TEXT("Sending %d/%d bytes (chunk %d)"), TotalSent, Ar.Num(), AmountSent);
		Remaining -= AmountSent;
		TotalSent += AmountSent;
	}
	check(Remaining == 0);
	return true;
}

bool FSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket)
{
	if (Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Attempting to read from a disconnected socket."));
		return false;
	}

	TArray<uint8> HeaderBytes;
	const int32 HeaderSize = sizeof(FSocketMessageHeader);
	HeaderBytes.AddZeroed(HeaderSize);

	if (!UCV::SocketUtils::SocketReceiveAll(Socket, HeaderBytes.GetData(), HeaderSize))
	{
		UE_LOG(LogUnrealCV, Log, TEXT("Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic = 0;
	Reader << Magic;

	if (Magic != DefaultMagic)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Bad header magic (got 0x%08X, expected 0x%08X)."), Magic, DefaultMagic);
		return false;
	}

	uint32 PayloadSize = 0;
	Reader << PayloadSize;
	if (PayloadSize == 0)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Received empty payload."));
		return false;
	}
	if (PayloadSize > MaxPayloadSizeBytes)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Payload too large: %u bytes."), PayloadSize);
		return false;
	}

	const int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!UCV::SocketUtils::SocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Incomplete payload read — socket disconnected."));
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// UTcpServer
// ---------------------------------------------------------------------------

bool UTcpServer::IsConnected() const
{
	return ConnectionSocket != nullptr;
}

void UTcpServer::CleanupConnection()
{
	if (ConnectionSocket)
	{
		ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		ConnectionSocket = nullptr;
	}
}

UTcpServer::~UTcpServer()
{
	CleanupConnection();
	if (TcpListener.IsValid())
	{
		TcpListener->Stop();
		TcpListener.Reset();
	}
}

bool UTcpServer::StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (ConnectionSocket) { return false; }

	UE_LOG(LogUnrealCV, Warning, TEXT("Echo service: client from %s"), *ClientEndpoint.ToString());
	ConnectionSocket = ClientSocket;

	constexpr uint32 BufferSize = 1024;
	TArray<uint8> ReceivedData;
	ReceivedData.SetNumZeroed(BufferSize);

	while (true)
	{
		int32 BytesRead = 0;
		ClientSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), BytesRead);
		if (BytesRead <= 0) { ConnectionSocket = nullptr; return false; }

		int32 TotalSent = 0;
		while (TotalSent < BytesRead)
		{
			int32 BytesSent = 0;
			ClientSocket->Send(ReceivedData.GetData() + TotalSent, BytesRead - TotalSent, BytesSent);
			if (BytesSent <= 0)
			{
				ConnectionSocket = nullptr;
				return false;
			}
			TotalSent += BytesSent;
		}
	}
}

bool UTcpServer::StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Rejecting connection from %s — only one client allowed."), *ClientEndpoint.ToString());
		return false;
	}

	ConnectionSocket = ClientSocket;
	UE_LOG(LogUnrealCV, Display, TEXT("New client connected from %s"), *ClientEndpoint.ToString());

	const FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
	if (!SendMessage(Confirm))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to send welcome message."));
	}

	while (ConnectionSocket)
	{
		if (ConnectionSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Socket reports disconnected state."));
		}

		FArrayReader ArrayReader;
		if (!FSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket))
		{
			CleanupConnection();
			return false;
		}

		const FString Message = UCV::SocketUtils::StringFromBinaryArray(ArrayReader);

		TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
		const FString Endpoint = EndpointAddr->ToString(true);
		ReceivedEvent.Broadcast(Endpoint, Message);
	}
	return false;
}

bool UTcpServer::OnClientConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	ConnectedEvent.Broadcast(*ClientEndpoint.ToString());
	return StartMessageService(ClientSocket, ClientEndpoint);
}

bool UTcpServer::Start(int32 InPortNum)
{
	if (InPortNum == PortNum && bIsListening) { return true; }

	CleanupConnection();
	if (TcpListener.IsValid())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Stopping previous TCP listener."));
		TcpListener->Stop();
	}

	PortNum = InPortNum;
	const FIPv4Endpoint Endpoint(FIPv4Address(0, 0, 0, 0), PortNum);

	constexpr int32 MaxConnections = 1;
	FSocket* ServerSocket = FTcpSocketBuilder(TEXT("UnrealCV-TcpListener"))
		.BoundToEndpoint(Endpoint)
		.Listening(MaxConnections);

	if (!ServerSocket)
	{
		bIsListening = false;
		UE_LOG(LogUnrealCV, Warning, TEXT("Cannot listen on port %d — port may be in use."), PortNum);
		return false;
	}

	int32 NewBufferSize = 0;
	ServerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewBufferSize);

	TcpListener = MakeShared<FTcpListener>(*ServerSocket);
	TcpListener->OnConnectionAccepted().BindUObject(this, &UTcpServer::OnClientConnected);

	if (!TcpListener->Init())
	{
		bIsListening = false;
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to initialise TCP listener on port %d."), PortNum);
		return false;
	}

	bIsListening = true;
	UE_LOG(LogUnrealCV, Display, TEXT("Listening on port %d."), PortNum);
	return true;
}

bool UTcpServer::SendMessage(const FString& Message)
{
	if (!ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("SendMessage: no client connected."));
		return false;
	}

	TArray<uint8> Payload;
	UCV::SocketUtils::BinaryArrayFromString(Message, Payload);
	UE_LOG(LogUnrealCV, Verbose, TEXT("Sending string message (%d bytes)."), Payload.Num());
	return FSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

bool UTcpServer::SendData(const TArray<uint8>& Payload)
{
	if (!ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("SendData: no client connected."));
		return false;
	}

	UE_LOG(LogUnrealCV, Verbose, TEXT("Sending binary payload (%d bytes)."), Payload.Num());
	return FSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

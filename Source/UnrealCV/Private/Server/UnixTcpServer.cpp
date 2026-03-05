// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "UnixTcpServer.h"
#include "SocketUtils.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "HAL/PlatformProcess.h"
#include "UnrealcvLog.h"
#include "UnrealcvShim.h"
#include <string>

#if PLATFORM_LINUX
#include <cstdlib>
#include <cstddef>
#include <sys/socket.h>
#include <sys/un.h>
#include <cstring>
#include <unistd.h>
#endif

uint32 FUnixSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

namespace
{
constexpr uint32 MaxPayloadSizeBytes = 256u * 1024u * 1024u;
}

// ---------------------------------------------------------------------------
// TCP transport
// ---------------------------------------------------------------------------

bool FUnixSocketMessageHeader::WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FUnixSocketMessageHeader Header(Payload);
	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 TotalSent = 0, Remaining = Ar.Num(), RetriesLeft = 100;
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

bool FUnixSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket)
{
	if (Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Attempting to read from a disconnected socket."));
		return false;
	}

	TArray<uint8> HeaderBytes;
	const int32 HeaderSize = sizeof(FUnixSocketMessageHeader);
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
		UE_LOG(LogUnrealCV, Error, TEXT("Bad header magic (got 0x%08X)."), Magic);
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
		UE_LOG(LogUnrealCV, Error, TEXT("Incomplete payload — socket disconnected."));
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
// UDS transport
// ---------------------------------------------------------------------------

bool FUnixSocketMessageHeader::WrapAndSendPayloadUDS(const TArray<uint8>& Payload, int Fd)
{
#if PLATFORM_LINUX
	FUnixSocketMessageHeader Header(Payload);
	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 TotalSent = 0, Remaining = Ar.Num(), RetriesLeft = 1000;
	while (Remaining > 0)
	{
		const int32 Written = static_cast<int32>(write(Fd, Ar.GetData() + TotalSent, Remaining));
		if (Written == -1)
		{
			if (--RetriesLeft < 0)
			{
				UE_LOG(LogUnrealCV, Error, TEXT("UDS send exhausted retries. Expected %d, sent %d."), Ar.Num(), TotalSent);
				return false;
			}
			FPlatformProcess::Sleep(0.001f);
			continue;
		}
		Remaining -= Written;
		TotalSent += Written;
	}
	check(Remaining == 0);
	return true;
#else
	return false;
#endif
}

bool FUnixSocketMessageHeader::ReceivePayloadUDS(FArrayReader& OutPayload, int Fd)
{
#if PLATFORM_LINUX
	TArray<uint8> HeaderBytes;
	const int32 HeaderSize = sizeof(FUnixSocketMessageHeader);
	HeaderBytes.AddZeroed(HeaderSize);
	if (!UCV::SocketUtils::UDSReceiveAll(Fd, HeaderBytes.GetData(), HeaderSize))
	{
		UE_LOG(LogUnrealCV, Log, TEXT("UDS client disconnected (header read failed)."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic = 0;
	Reader << Magic;
	if (Magic != DefaultMagic)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Bad UDS header magic (got 0x%08X)."), Magic);
		return false;
	}

	uint32 PayloadSize = 0;
	Reader << PayloadSize;
	if (PayloadSize == 0)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UDS empty payload."));
		return false;
	}
	if (PayloadSize > MaxPayloadSizeBytes)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UDS payload too large: %u bytes."), PayloadSize);
		return false;
	}

	const int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!UCV::SocketUtils::UDSReceiveAll(Fd, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Incomplete UDS payload — client disconnected."));
		return false;
	}
	return true;
#else
	return false;
#endif
}

// ---------------------------------------------------------------------------
// UUnixTcpServer
// ---------------------------------------------------------------------------

bool UUnixTcpServer::IsConnected() const
{
	return ConnectionSocket != nullptr;
}

void UUnixTcpServer::CleanupConnection()
{
	if (ConnectionSocket)
	{
		ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
		ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		ConnectionSocket = nullptr;
	}
}

UUnixTcpServer::~UUnixTcpServer()
{
	CleanupConnection();
	if (TcpListener.IsValid())
	{
		TcpListener->Stop();
		TcpListener.Reset();
	}
#if PLATFORM_LINUX
	if (UDS_ConnFd != -1)   { shutdown(UDS_ConnFd, SHUT_RDWR);   close(UDS_ConnFd);   UDS_ConnFd = -1; }
	if (UDS_ListenFd != -1) { shutdown(UDS_ListenFd, SHUT_RDWR); close(UDS_ListenFd); UDS_ListenFd = -1; }
	// Clean up the socket file to avoid stale sockets.
	if (PortNum > 0)
	{
		const std::string SocketPath = "/tmp/unrealcv_" + std::to_string(PortNum) + ".socket";
		unlink(SocketPath.c_str());
	}
#endif
}

bool UUnixTcpServer::StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
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

bool UUnixTcpServer::StartMessageServiceUDS()
{
#if PLATFORM_LINUX
	const std::string SocketPath = "/tmp/unrealcv_" + std::to_string(PortNum) + ".socket";
	UE_LOG(LogUnrealCV, Log, TEXT("UDS server socket: %hs"), SocketPath.c_str());

	struct sockaddr_un ServerAddr;
	FMemory::Memzero(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sun_family = AF_UNIX;
	FCStringAnsi::Strncpy(ServerAddr.sun_path, SocketPath.c_str(), sizeof(ServerAddr.sun_path) - 1);

	const int ListenFd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (ListenFd < 0)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UDS socket() failed: %hs"), strerror(errno));
		return false;
	}

	unlink(SocketPath.c_str());
	const socklen_t AddrLen = offsetof(struct sockaddr_un, sun_path) + strlen(ServerAddr.sun_path);
	if (bind(ListenFd, reinterpret_cast<struct sockaddr*>(&ServerAddr), AddrLen) < 0)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UDS bind() failed: %hs"), strerror(errno));
		close(ListenFd);
		return false;
	}
	if (listen(ListenFd, 1) < 0)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UDS listen() failed: %hs"), strerror(errno));
		close(ListenFd);
		return false;
	}

	UDS_ListenFd = ListenFd;
	UE_LOG(LogUnrealCV, Log, TEXT("UDS listening on %hs"), SocketPath.c_str());

	while (true)
	{
		UE_LOG(LogUnrealCV, Log, TEXT("UDS accepting connections..."));
		struct sockaddr_un ClientAddr;
		socklen_t ClientLen = sizeof(ClientAddr);
		const int ConnFd = accept(UDS_ListenFd, reinterpret_cast<struct sockaddr*>(&ClientAddr), &ClientLen);
		if (ConnFd < 0) { UE_LOG(LogUnrealCV, Error, TEXT("UDS accept() failed.")); break; }

		UDS_ConnFd = ConnFd;
		UE_LOG(LogUnrealCV, Display, TEXT("UDS client connected."));

		const FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
		if (!SendMessageUDS(Confirm))
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to send UDS welcome message."));
			UDS_ConnFd = -1;
			continue;
		}

		while (UDS_ConnFd != -1)
		{
			FArrayReader ArrayReader;
			if (!FUnixSocketMessageHeader::ReceivePayloadUDS(ArrayReader, UDS_ConnFd))
			{
				UDS_ConnFd = -1;
				break;
			}
			ReceivedEvent.Broadcast(TEXT("UDS client"), UCV::SocketUtils::StringFromBinaryArray(ArrayReader));
		}
		UE_LOG(LogUnrealCV, Display, TEXT("UDS client disconnected — waiting for new connections."));
	}

	close(UDS_ListenFd);
	UDS_ListenFd = -1;
	UDS_ConnFd = -1;
	UE_LOG(LogUnrealCV, Display, TEXT("UDS server shut down."));
#endif
	return false;
}

bool UUnixTcpServer::StartMessageServiceINet(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Rejecting %s — only one client allowed."), *ClientEndpoint.ToString());
		return false;
	}

	ConnectionSocket = ClientSocket;
	UE_LOG(LogUnrealCV, Display, TEXT("New TCP client from %s"), *ClientEndpoint.ToString());

	const FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
	if (!SendMessageINet(Confirm))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to send TCP welcome message."));
	}

	while (ConnectionSocket)
	{
		if (ConnectionSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Socket reports disconnected."));
		}

		FArrayReader ArrayReader;
		if (!FUnixSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket))
		{
			if (ConnectionSocket)
			{
				ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
				ConnectionSocket->Close();
				ConnectionSocket = nullptr;
			}
			return false;
		}

		TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
		ReceivedEvent.Broadcast(EndpointAddr->ToString(true), UCV::SocketUtils::StringFromBinaryArray(ArrayReader));
	}
	return false;
}

/**
 * Connection callback from the TCP listener.
 * Handles the TCP phase first, then transitions to UDS on Linux.
 */
bool UUnixTcpServer::OnClientConnected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	ConnectedEvent.Broadcast(*ClientEndpoint.ToString());
	const bool Status = StartMessageServiceINet(ClientSocket, ClientEndpoint);
	bIsUDS.Store(true);
	StartMessageServiceUDS();
	return Status;
}

bool UUnixTcpServer::Start(int32 InPortNum)
{
	if (InPortNum == PortNum && bIsListening) { return true; }

	CleanupConnection();
	if (TcpListener.IsValid())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Stopping previous listener."));
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
	TcpListener->OnConnectionAccepted().BindUObject(this, &UUnixTcpServer::OnClientConnected);
	if (!TcpListener->Init())
	{
		bIsListening = false;
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to init TCP listener on port %d."), PortNum);
		return false;
	}

	bIsListening = true;
	UE_LOG(LogUnrealCV, Display, TEXT("Listening on port %d."), PortNum);
	return true;
}

bool UUnixTcpServer::SendMessage(const FString& Message)
{
#if PLATFORM_LINUX
	return bIsUDS.Load() ? SendMessageUDS(Message) : SendMessageINet(Message);
#else
	return SendMessageINet(Message);
#endif
}

bool UUnixTcpServer::SendData(const TArray<uint8>& Payload)
{
#if PLATFORM_LINUX
	return bIsUDS.Load() ? SendDataUDS(Payload) : SendDataINet(Payload);
#else
	return SendDataINet(Payload);
#endif
}

bool UUnixTcpServer::SendMessageINet(const FString& Message)
{
	if (!ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("SendMessageINet: no client connected."));
		return false;
	}
	TArray<uint8> Payload;
	UCV::SocketUtils::BinaryArrayFromString(Message, Payload);
	UE_LOG(LogUnrealCV, Verbose, TEXT("TCP sending string (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

bool UUnixTcpServer::SendDataINet(const TArray<uint8>& Payload)
{
	if (!ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("SendDataINet: no client connected."));
		return false;
	}
	UE_LOG(LogUnrealCV, Verbose, TEXT("TCP sending binary (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

bool UUnixTcpServer::SendMessageUDS(const FString& Message)
{
#if PLATFORM_LINUX
	if (UDS_ConnFd == -1) { UE_LOG(LogUnrealCV, Error, TEXT("UDS fd invalid.")); return false; }
	TArray<uint8> Payload;
	UCV::SocketUtils::BinaryArrayFromString(Message, Payload);
	UE_LOG(LogUnrealCV, Verbose, TEXT("UDS sending string (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_ConnFd);
#else
	return false;
#endif
}

bool UUnixTcpServer::SendDataUDS(const TArray<uint8>& Payload)
{
#if PLATFORM_LINUX
	if (UDS_ConnFd == -1) { UE_LOG(LogUnrealCV, Error, TEXT("UDS fd invalid.")); return false; }
	UE_LOG(LogUnrealCV, Verbose, TEXT("UDS sending binary (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_ConnFd);
#else
	return false;
#endif
}

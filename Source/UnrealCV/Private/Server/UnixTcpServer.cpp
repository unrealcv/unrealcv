// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "UnixTcpServer.h"
#include "Runtime/Core/Public/Serialization/BufferArchive.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"
#include <string>
#include "UnrealcvLog.h"
#include "UnrealcvShim.h"

uint32 FUnixSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

namespace
{

FString StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	const std::string Utf8(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(UTF8_TO_TCHAR(Utf8.c_str()));
}

void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	auto Converter = StringCast<UTF8CHAR>(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
}

bool SocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize)
{
	int32 Offset = 0;
	while (ExpectedSize > 0)
	{
		int32 NumRead = 0;
		Socket->Recv(Result + Offset, ExpectedSize, NumRead);
		check(NumRead <= ExpectedSize);

		const ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
		if (NumRead > 0) { Offset += NumRead; ExpectedSize -= NumRead; continue; }
		if (LastError == ESocketErrors::SE_EWOULDBLOCK) { continue; }
		if (LastError == ESocketErrors::SE_NO_ERROR)
		{
			UE_LOG(LogUnrealCV, Log, TEXT("Connection gracefully closed by the client."));
			return false;
		}
		if (LastError == ESocketErrors::SE_ECONNABORTED)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Connection aborted unexpectedly."));
			return false;
		}
		if (LastError == ESocketErrors::SE_ENOTCONN)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Socket is not connected."));
			return false;
		}
		const TCHAR* ErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
		UE_LOG(LogUnrealCV, Error, TEXT("Unexpected socket error: %s"), ErrorMsg);
		return false;
	}
	return true;
}

#if PLATFORM_LINUX
bool UDSReceiveAll(int Fd, uint8* Result, int32 ExpectedSize)
{
	int32 Offset = 0;
	while (ExpectedSize > 0)
	{
		const int32 NumRead = static_cast<int32>(read(Fd, Result + Offset, ExpectedSize));
		check(NumRead <= ExpectedSize);

		if (NumRead > 0) { Offset += NumRead; ExpectedSize -= NumRead; continue; }
		if (NumRead == 0)
		{
			UE_LOG(LogUnrealCV, Log, TEXT("UDS connection gracefully closed."));
			close(Fd);
			return false;
		}
		UE_LOG(LogUnrealCV, Error, TEXT("UDS read error: %hs"), strerror(errno));
		close(Fd);
		return false;
	}
	return true;
}
#endif

} // anonymous namespace

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
	}

	TArray<uint8> HeaderBytes;
	const int32 HeaderSize = sizeof(FUnixSocketMessageHeader);
	HeaderBytes.AddZeroed(HeaderSize);
	if (!SocketReceiveAll(Socket, HeaderBytes.GetData(), HeaderSize))
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

	const int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!SocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize))
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
			UE_LOG(LogUnrealCV, Error, TEXT("UDS write error: %hs"), strerror(errno));
			close(Fd);
			return false;
		}
		if (--RetriesLeft < 0)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("UDS send exhausted retries. Expected %d, sent %d."), Ar.Num(), TotalSent);
			close(Fd);
			return false;
		}
		Remaining -= Written;
		TotalSent += Written;
	}
	check(Remaining == 0);
#endif
	return true;
}

bool FUnixSocketMessageHeader::ReceivePayloadUDS(FArrayReader& OutPayload, int Fd)
{
#if PLATFORM_LINUX
	TArray<uint8> HeaderBytes;
	const int32 HeaderSize = sizeof(FUnixSocketMessageHeader);
	HeaderBytes.AddZeroed(HeaderSize);
	if (!UDSReceiveAll(Fd, HeaderBytes.GetData(), HeaderSize))
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

	const int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!UDSReceiveAll(Fd, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Incomplete UDS payload — client disconnected."));
		return false;
	}
#endif
	return true;
}

// ---------------------------------------------------------------------------
// UUnixTcpServer
// ---------------------------------------------------------------------------

bool UUnixTcpServer::IsConnected() const
{
	return ConnectionSocket != nullptr;
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
		if (BytesRead == 0) { ConnectionSocket = nullptr; return false; }
		int32 BytesSent = 0;
		ClientSocket->Send(ReceivedData.GetData(), BytesRead, BytesSent);
		check(BytesRead == BytesSent);
	}
}

bool UUnixTcpServer::StartMessageServiceUDS()
{
#if PLATFORM_LINUX
	if (UDS_ConnFd != -1)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("UDS: only one client allowed."));
		return false;
	}

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
	if (listen(ListenFd, 5) < 0)
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
			BroadcastReceived(TEXT("UDS client"), StringFromBinaryArray(ArrayReader));
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
			ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			ConnectionSocket->Close();
			ConnectionSocket = nullptr;
			return false;
		}

		TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
		BroadcastReceived(EndpointAddr->ToString(true), StringFromBinaryArray(ArrayReader));
	}
	return false;
}

bool UUnixTcpServer::Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	BroadcastConnected(*ClientEndpoint.ToString());
	const bool Status = StartMessageServiceINet(ClientSocket, ClientEndpoint);
	bIsUDS = true;
	StartMessageServiceUDS();
	return Status;
}

bool UUnixTcpServer::Start(int32 InPortNum)
{
	if (InPortNum == PortNum && bIsListening) { return true; }

	if (ConnectionSocket) { ConnectionSocket->Close(); ConnectionSocket = nullptr; }
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
	TcpListener->OnConnectionAccepted().BindUObject(this, &UUnixTcpServer::Connected);
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
	return bIsUDS ? SendMessageUDS(Message) : SendMessageINet(Message);
#else
	return SendMessageINet(Message);
#endif
}

bool UUnixTcpServer::SendData(const TArray<uint8>& Payload)
{
#if PLATFORM_LINUX
	return bIsUDS ? SendDataUDS(Payload) : SendDataINet(Payload);
#else
	return SendDataINet(Payload);
#endif
}

bool UUnixTcpServer::SendMessageINet(const FString& Message)
{
	if (!ConnectionSocket) { return false; }
	TArray<uint8> Payload;
	BinaryArrayFromString(Message, Payload);
	UE_LOG(LogUnrealCV, Verbose, TEXT("TCP sending string (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

bool UUnixTcpServer::SendDataINet(const TArray<uint8>& Payload)
{
	if (!ConnectionSocket) { return false; }
	UE_LOG(LogUnrealCV, Verbose, TEXT("TCP sending binary (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
}

bool UUnixTcpServer::SendMessageUDS(const FString& Message)
{
	if (UDS_ConnFd == -1) { UE_LOG(LogUnrealCV, Error, TEXT("UDS fd invalid.")); return false; }
	TArray<uint8> Payload;
	BinaryArrayFromString(Message, Payload);
	UE_LOG(LogUnrealCV, Verbose, TEXT("UDS sending string (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_ConnFd);
}

bool UUnixTcpServer::SendDataUDS(const TArray<uint8>& Payload)
{
	if (UDS_ConnFd == -1) { UE_LOG(LogUnrealCV, Error, TEXT("UDS fd invalid.")); return false; }
	UE_LOG(LogUnrealCV, Verbose, TEXT("UDS sending binary (%d bytes)."), Payload.Num());
	return FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_ConnFd);
}

UUnixTcpServer::~UUnixTcpServer()
{
#if PLATFORM_LINUX
	if (UDS_ConnFd != -1) { shutdown(UDS_ConnFd, SHUT_RDWR); close(UDS_ConnFd); UDS_ConnFd = -1; }
	if (UDS_ListenFd != -1) { shutdown(UDS_ListenFd, SHUT_RDWR); close(UDS_ListenFd); UDS_ListenFd = -1; }
#endif
}

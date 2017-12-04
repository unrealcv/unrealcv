#include "UnrealCVPrivate.h"
#include "TcpServer.h"
#include "Networking.h"
#include <string>

uint32 FSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

bool FSocketMessageHeader::WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FSocketMessageHeader Header(Payload);

	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 TotalAmountSent = 0; // How many bytes have been sent
    int32 AmountToSend = Ar.Num();
    int NumTrial = 100; // Only try a limited amount of times
    // int ChunkSize = 4096;
    while (AmountToSend > 0)
    {
        int AmountSent = 0;
        // GetData returns a uint8 pointer
        Socket->Send(Ar.GetData() + TotalAmountSent, Ar.Num() - TotalAmountSent, AmountSent);
        NumTrial--;

        if (AmountSent == -1)
        {
            continue;
        }

        if (NumTrial < 0)
        {
            UE_LOG(LogUnrealCV, Error, TEXT("Unable to send. Expect to send %d, sent %d"), Ar.Num(), TotalAmountSent);
            return false;
        }

        UE_LOG(LogUnrealCV, Verbose, TEXT("Sending bytes %d/%d, sent %d"), TotalAmountSent, Ar.Num(), AmountSent);
        AmountToSend -= AmountSent;
        TotalAmountSent += AmountSent;
    }
    check(AmountToSend == 0);
	return true;
}

/* Waiting for data, return false only when disconnected */
bool SocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize, bool* unknown_error)
{
	int32 Offset = 0;
	while (ExpectedSize > 0)
	{
		// uint32 PendingDataSize;
		// bool Status = Socket->HasPendingData(PendingDataSize);
		// status = PendingDataSize != 0
		int32 NumRead = 0;
		bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead);
		// bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead, ESocketReceiveFlags::WaitAll);
		// WaitAll is not effective for non-blocking socket, see here https://msdn.microsoft.com/en-us/library/windows/desktop/ms740121(v=vs.85).aspx
		// Check pending data first, see https://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx

		// ESocketConnectionState ConnectionState = Socket->GetConnectionState();
		// check(NumRead <= ExpectedSize);
		// RecvStatus == BytesRead >= 0
		check(NumRead <= ExpectedSize);

		ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
		// Use this check instead of use return status of recv to ensure backward compatibility
		// Because there is a bug with 4.12 FSocketBSD implementation
		// https://www.unrealengine.com/blog/unreal-engine-4-13-released, Search FSocketBSD::Recv
		if (NumRead == 0 && LastError == ESocketErrors::SE_NO_ERROR) // 0 means gracefully closed
		{
			UE_LOG(LogUnrealCV, Log, TEXT("The connection is gracefully closed by the client."));
			return false; // Socket is disconnected. if -1, keep waiting for data
		}

		if (NumRead == 0 && LastError == ESocketErrors::SE_EWOULDBLOCK)
		{
			continue; // No data and keep waiting
		}

		if (LastError == ESocketErrors::SE_NO_ERROR || LastError == ESocketErrors::SE_EWOULDBLOCK)
		{
			// Got some data and in an expected condition
			Offset += NumRead;
			ExpectedSize -= NumRead;
			continue;
		}
		// LastError == ESocketErrors::SE_EWOULDBLOCK means running a non-block socket."));
		if (LastError == ESocketErrors::SE_ECONNABORTED) // SE_ECONNABORTED
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Connection aborted unexpectly."));
			return false;
		}

		const TCHAR* LastErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
		UE_LOG(LogUnrealCV, Error, TEXT("Unexpected error of socket happend, error %s"), LastErrorMsg);
    if (unknown_error != nullptr) {
      *unknown_error = true;
    }
		return false;
	}
	return true;
}


bool FSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket, bool* unknown_error)
{
	TArray<uint8> HeaderBytes;
	int32 Size = sizeof(FSocketMessageHeader);
	HeaderBytes.AddZeroed(Size);

	if (!SocketReceiveAll(Socket, HeaderBytes.GetData(), Size, unknown_error))
	{
		// false here means socket disconnected.
		// UE_LOG(LogUnrealCV, Error, TEXT("Unable to read header, Socket disconnected."));
		UE_LOG(LogUnrealCV, Log, TEXT("Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic;
	Reader << Magic;

	if (Magic != FSocketMessageHeader::DefaultMagic)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Bad network header magic"));
		return false;
	}

	uint32 PayloadSize;
	Reader << PayloadSize;
	if (!PayloadSize)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Empty payload"));
		return false;
	}

	int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!SocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize, unknown_error))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unable to read full payload, Socket disconnected."));
		return false;
	}

	// Skip CRC checking in FNFSMessageHeader
	return true;
}

// TODO: Wrap these two functions into a class
FString StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	FTCHARToUTF8 Convert(*Message);

	OutBinaryArray.Empty();

	// const TArray<TCHAR>& CharArray = Message.GetCharArray();
	// OutBinaryArray.Append(CharArray);
	// This can work, but will add tailing \0 also behavior is not well defined.

	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
}


bool UNetworkManager::IsConnected()
{
	return (this->ConnectionSocket != NULL);
}

/* Provide a dummy echo service to echo received data back for development purpose */
bool UNetworkManager::StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (!this->ConnectionSocket) // Only maintain one active connection, So just reuse the TCPListener thread.
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("New client connected from %s"), *ClientEndpoint.ToString());
		// ClientSocket->SetNonBlocking(false); // When this in blocking state, I can not use this socket to send message back
		ConnectionSocket = ClientSocket;

		// Listening data here or start a new thread for data?
		// Reuse the TCP Listener thread for getting data, only support one connection
		uint32 BufferSize = 1024;
		int32 Read = 0;
		TArray<uint8> ReceivedData;
		ReceivedData.SetNumZeroed(BufferSize);
		while (1)
		{
			// Easier to use raw FSocket here, need to detect remote socket disconnection
			bool RecvStatus = ClientSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

			// if (!RecvStatus) // The connection is broken
			if (Read == 0)
			// RecvStatus == true if Read >= 0, this is used to determine client disconnection
			// -1 means no data, 0 means disconnected
			{
				ConnectionSocket = NULL; // Use this to determine whether client is connected
				return false;
			}
			int32 Sent;
			ClientSocket->Send(ReceivedData.GetData(), Read, Sent); // Echo the message back
			check(Read == Sent);
		}
		return true;
	}
	return false;
}

/**
  * Start message service in listening thread
  * TODO: Start a new background thread to receive message
  */
bool UNetworkManager::StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (!this->ConnectionSocket)
	{
		ConnectionSocket = ClientSocket;

		UE_LOG(LogUnrealCV, Warning, TEXT("New client connected from %s"), *ClientEndpoint.ToString());
		// ClientSocket->SetNonBlocking(false); // When this in blocking state, I can not use this socket to send message back
		FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
		bool IsSent = this->SendMessage(Confirm); // Send a hello message
		if (!IsSent)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to send welcome message to client."));
		}

		// TODO: Start a new thread
		while (this->ConnectionSocket) // Listening thread, while the client is still connected
		{
			FArrayReader ArrayReader;
      bool unknown_error = false;
			if (!FSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket, &unknown_error))
				// Wait forever until got a message, or return false when error happened
			{
        if (unknown_error) {
          BroadcastError(FString("ReceivePayload failed with unknown error"));
        }
				this->ConnectionSocket = NULL;
				return false; // false will release the ClientSocket
				break; // Remote socket disconnected
			}

			FString Message = StringFromBinaryArray(ArrayReader);
			BroadcastReceived(Message);
			// Fire raw message received event, use message id to connect request and response
			UE_LOG(LogUnrealCV, Warning, TEXT("Receive message %s"), *Message);
		}
		return false; // TODO: What is the meaning of return value?
	}
	else
	{
		// No response and let the client silently timeout
		UE_LOG(LogUnrealCV, Warning, TEXT("Only one client is allowed, can not allow new connection from %s"), *ClientEndpoint.ToString());
		return false; // Already have a connection
	}
}

/** Connected Handler */
bool UNetworkManager::Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	bool ServiceStatus = false;
	BroadcastConnected(*ClientEndpoint.ToString());
	// ServiceStatus = StartEchoService(ClientSocket, ClientEndpoint);
	ServiceStatus = StartMessageService(ClientSocket, ClientEndpoint);
	return ServiceStatus;
	// This is a blocking service, if need to support multiple connections, consider start a new thread here.
}


bool UNetworkManager::Start(int32 InPortNum) // Restart the server if configuration changed
{
	if (InPortNum == this->PortNum && this->bIsListening) return true; // Already started

	if (ConnectionSocket) // Release previous connection
	{
		ConnectionSocket->Close();
		ConnectionSocket = NULL;
	}

	if (TcpListener) // Delete previous configuration first
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Stop previous server"));
		TcpListener->Stop(); // TODO: test the robustness, will this operation successful?
		delete TcpListener;
	}

	this->PortNum = InPortNum; // Start a new TCPListener
	FIPv4Address IPAddress = FIPv4Address(0, 0, 0, 0);
	// int32 PortNum = this->PortNum; // Make this configuable
	FIPv4Endpoint Endpoint(IPAddress, PortNum);

	FSocket* ServerSocket = FTcpSocketBuilder(TEXT("FTcpListener server")) // TODO: Need to realease this socket
		// .AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);

	if (ServerSocket)
	{
		int32 NewSize = 0;
		ServerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);
	}
	else
	{
		this->bIsListening = false;
		UE_LOG(LogUnrealCV, Warning, TEXT("Cannot start listening on port %d, Port might be in use"), PortNum);
		return false;
	}

	TcpListener = new FTcpListener(*ServerSocket);
	// TcpListener = new FTcpListener(Endpoint); // This will be released after start
	// In FSocket, when a FSocket is set as reusable, it means SO_REUSEADDR, not SO_REUSEPORT.  see SocketsBSD.cpp
	TcpListener->OnConnectionAccepted().BindUObject(this, &UNetworkManager::Connected);
	if (TcpListener->Init())
	{
		this->bIsListening = true;
		UE_LOG(LogUnrealCV, Warning, TEXT("Start listening on %d"), PortNum);
		return true;
	}
	else
	{
		this->bIsListening = false;
		UE_LOG(LogUnrealCV, Error, TEXT("Can not start listening on port %d"), PortNum);
		return false;
	}
}

bool UNetworkManager::SendMessage(const FString& Message)
{
	if (ConnectionSocket)
	{
		TArray<uint8> Payload;
		BinaryArrayFromString(Message, Payload);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send string message with size %d"), Payload.Num());
		FSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"), Payload.Num());
		return true;
	}
	return false;
}

bool UNetworkManager::SendData(const TArray<uint8>& Payload)
{
	if (ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send binary payload with size %d"), Payload.Num());
		FSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"), Payload.Num());
		return true;
	}
	return false;
}

UNetworkManager::~UNetworkManager()
{
	if (ConnectionSocket)
	{
		ConnectionSocket->Close();
	}
	delete TcpListener;
}

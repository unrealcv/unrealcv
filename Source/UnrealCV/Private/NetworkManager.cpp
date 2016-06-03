// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "NetworkManager.h"
#include "Networking.h"
#include <string>

/*
class FMessageService : public FRunnable
{
public:
	FMessageService()
	{
		Thread = FRunnableThread::Create(this, TEXT("FMessageService"), 8 * 1024, TPri_Normal);
	}

	~FMessageService()
	{
		if (Thread != nullptr)
		{
			Thread->Kill(true);
			delete Thread;
		}
	}
};
*/

uint32 FSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

bool FSocketMessageHeader::WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FSocketMessageHeader Header(Payload);

	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 AmountSent;
	Socket->Send(Ar.GetData(), Ar.Num(), AmountSent);
	if (AmountSent != Ar.Num())
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to send."));
		return false;
	}
	return true;
}

/* Waiting for data, return false only when disconnected */
bool SocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize)
{
	int32 Offset = 0;
	while (ExpectedSize > 0)
	{
		// uint32 PendingDataSize;
		// bool Status = Socket->HasPendingData(PendingDataSize);
		/*
		if (!Status)
		{
			return false;
		}// Operation failed
		if (PendingDataSize == 0)
		{
			continue;
		}
		*/

		// if PendingDataSize != 0
		int32 NumRead = 0;
		bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead);
		// bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead, ESocketReceiveFlags::WaitAll);
		// WaitAll is not effective for non-blocking socket, see here https://msdn.microsoft.com/en-us/library/windows/desktop/ms740121(v=vs.85).aspx
		// Check pending data first, see https://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx

		// ESocketConnectionState ConnectionState = Socket->GetConnectionState();
		// check(NumRead <= ExpectedSize);
		// RecvStatus == BytesRead >= 0
		check(NumRead <= ExpectedSize);

		if (NumRead == 0) // 0 means gracefully closed 
		{
			return false; // Socket is disconnected. if -1, keep waiting for data
		}

		if (NumRead == -1) 
		// -1 means error happen, but not sure what the error is, in windows WSAGetLastError can get the real error
		// HasPendingData already eliminate the error of not having data
		{
			ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
			if (LastError == ESocketErrors::SE_EWOULDBLOCK)
			{
				continue; // No data received
			}
			else // SE_ECONNABORTED
			{
				return false;
			}
		}
		Offset += NumRead;
		ExpectedSize -= NumRead;
	}
	return true;
}


bool FSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket)
{
	TArray<uint8> HeaderBytes;
	int32 Size = sizeof(FSocketMessageHeader);
	HeaderBytes.AddZeroed(Size);

	if (!SocketReceiveAll(Socket, HeaderBytes.GetData(), Size))
	{
		// false here means socket disconnected.
		// UE_LOG(LogTemp, Error, TEXT("Unable to read header, Socket disconnected."));
		UE_LOG(LogTemp, Warning, TEXT("Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic;
	Reader << Magic;

	if (Magic != FSocketMessageHeader::DefaultMagic)
	{
		UE_LOG(LogTemp, Error, TEXT("Bad network header magic"));
		return false;
	}

	uint32 PayloadSize;
	Reader << PayloadSize;
	if (!PayloadSize)
	{
		UE_LOG(LogTemp, Error, TEXT("Empty payload"));
		return false;
	}

	int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);
	OutPayload.Seek(PayloadOffset);
	if (!SocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		// UE_LOG(LogTemp, Error, TEXT("Unable to read full payload"));
		UE_LOG(LogTemp, Error, TEXT("Unable to read full payload, Socket disconnected."));
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
	// This can work, but work add tailing \0 also behavior is not well defined.
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
		UE_LOG(LogTemp, Warning, TEXT("New client connected from %s"), *ClientEndpoint.ToString());
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
			if (Read == 0) // RecvStatus == true if Read >= 0, this is used to determine client disconnection
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

/** Start message service in listening thread
	* TODO: Start a new background thread to receive message
	*/
bool UNetworkManager::StartMessageService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (!this->ConnectionSocket)
	{
		ConnectionSocket = ClientSocket;

		UE_LOG(LogTemp, Warning, TEXT("New client connected from %s"), *ClientEndpoint.ToString());
		// ClientSocket->SetNonBlocking(false); // When this in blocking state, I can not use this socket to send message back
		FString Confirm = FString::Printf(TEXT("connected to %s"), FApp::GetGameName());
		this->SendMessage(Confirm); // Send a hello message

		// TODO: Start a new thread
		while (this->ConnectionSocket) // Listening thread, while the client is still connected
		{
			FArrayReader ArrayReader;
			if (!FSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket))
				// Wait forever until got a message, or return false when error happened
			{
				this->ConnectionSocket = NULL;
				return false; // false will release the ClientSocket
				break; // Remote socket disconnected
			}

			FString Message = StringFromBinaryArray(ArrayReader);
			BroadcastReceived(Message);
			// Fire raw message received event, use message id to connect request and response
			UE_LOG(LogTemp, Warning, TEXT("Receive message %s"), *Message);
		}
		return false; // TODO: What is the meaning of return value?
	}
	else
	{
		// No response and let the client silently timeout
		UE_LOG(LogTemp, Warning, TEXT("Only one client is allowed, can not allow new connection from %s"), *ClientEndpoint.ToString());
		return false; // Already have a connection
	}
}

bool UNetworkManager::Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	bool ServiceStatus = false;
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
		UE_LOG(LogTemp, Warning, TEXT("Stop previous server"));
		TcpListener->Stop(); // TODO: test the robustness, will this operation successful?
		delete TcpListener;
	}

	this->PortNum = InPortNum; // Start a new TCPListener
	FIPv4Address IPAddress = FIPv4Address(0, 0, 0, 0);
	// int32 PortNum = this->PortNum; // Make this configuable
	FIPv4Endpoint Endpoint(IPAddress, PortNum);

	TcpListener = new FTcpListener(Endpoint); // This will be released after start
	// In FSocket, when a FSocket is set as reusable, it means SO_REUSEADDR, not SO_REUSEPORT.  see SocketsBSD.cpp
	TcpListener->OnConnectionAccepted().BindUObject(this, &UNetworkManager::Connected);
	if (TcpListener->Init())
	{
		this->bIsListening = true;
		UE_LOG(LogTemp, Warning, TEXT("Start listening on %d"), PortNum);
		return true;
	}
	else
	{
		this->bIsListening = false;
		UE_LOG(LogTemp, Error, TEXT("Can not start listening on port %d"), PortNum);
		return false;
	}
}

bool UNetworkManager::SendMessage(const FString& Message)
{
	if (ConnectionSocket)
	{
		TArray<uint8> Payload;
		BinaryArrayFromString(Message, Payload);

		FSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
		return true;
	}
	return false;
}

UNetworkManager::~UNetworkManager()
{
	delete TcpListener;
}

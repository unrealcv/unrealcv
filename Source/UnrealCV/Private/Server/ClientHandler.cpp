#include "ClientHandler.h"

uint32 FMultiSocketMessageHeader::DefaultMagic = 0x9E2B83C1;
/* Waiting for data, return false only when disconnected */
bool MultiSocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize)
{
	// Make sure no error before using this socket
	// ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
	// const TCHAR* LastErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
	// UE_LOG(LogUnrealCV, Display, TEXT("Error msg before receiving data %s"), LastErrorMsg);

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

		if (NumRead != 0) // successfully read something
		{
			// Got some data and in an expected condition
			Offset += NumRead;
			ExpectedSize -= NumRead;
			continue;
		}
		else
		{
			if (LastError == ESocketErrors::SE_EWOULDBLOCK)
			{
				continue; // No data and keep waiting
			}
			// Use this check instead of use return status of recv to ensure backward compatibility
			// Because there is a bug with 4.12 FSocketBSD implementation
			// https://www.unrealengine.com/blog/unreal-engine-4-13-released, Search FSocketBSD::Recv
			if (LastError == ESocketErrors::SE_NO_ERROR) // 0 means gracefully closed
			{
				UE_LOG(LogUnrealCV, Log, TEXT("The connection is gracefully closed by the client."));
				return false; // Socket is disconnected. if -1, keep waiting for data
			}

			// LastError == ESocketErrors::SE_EWOULDBLOCK means running a non-block socket."));
			if (LastError == ESocketErrors::SE_ECONNABORTED) // SE_ECONNABORTED
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Connection aborted unexpectly."));
				return false;
			}

			if (LastError == ESocketErrors::SE_ENOTCONN)
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Socket is not connected."));
				return false;
			}

			const TCHAR* LastErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
			UE_LOG(LogUnrealCV, Error, TEXT("Unexpected error of socket happend, error %s"), LastErrorMsg);

			return false;
		}
	}
	return true;
}

bool FMultiSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket)
{
	if (Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Trying to read message from an unconnected socket."));
	}
	TArray<uint8> HeaderBytes;
	int32 Size = sizeof(FMultiSocketMessageHeader);
	HeaderBytes.AddZeroed(Size);

	if (!MultiSocketReceiveAll(Socket, HeaderBytes.GetData(), Size))
	{
		// false here means socket disconnected.
		// UE_LOG(LogUnrealCV, Error, TEXT("Unable to read header, Socket disconnected."));
		UE_LOG(LogUnrealCV, Log, TEXT("Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic;
	Reader << Magic;

	if (Magic != FMultiSocketMessageHeader::DefaultMagic)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Bad network header magic"));
		return false;
	}

	uint32 PayloadSize;
	Reader << PayloadSize;
	if (0 == PayloadSize)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Empty payload"));
		return false;
	}

	int32 PayloadOffset = OutPayload.AddUninitialized(PayloadSize);  // return Number of elements in array before addition.
	OutPayload.Seek(PayloadOffset);
	if (!MultiSocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unable to read full payload, Socket disconnected."));
		return false;
	}

	// Skip CRC checking in FNFSMessageHeader
	return true;
}

FString MultiStringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}


FClientHandler::FClientHandler(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{

	ConnectionSocket = ClientSocket;
	ThreadName = FString::Printf(TEXT("New client connected from %s"), *ClientEndpoint.ToString());
	Thread = FRunnableThread::Create(this, *ThreadName);
	UE_LOG(LogUnrealCV, Log, TEXT("Create a new thread!"));
}

FClientHandler::~FClientHandler()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

bool FClientHandler::Init()
{
	bStop = false;
	return true;
}

uint32 FClientHandler::Run()
{

	while (ConnectionSocket)
	{
		if (bStop)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Stop the thread!"));
			return 1;
		}

		if (ConnectionSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Trying to read message from an unconnected socket."));
		}

		FArrayReader ArrayReader;
		if (!FMultiSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket))
			// Wait forever until got a message, or return false when error happened
		{
			// FIX: important to close the connection, otherwise listening socket may refuse new connection
			// https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
			ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			ConnectionSocket->Close();
			ConnectionSocket = NULL;
			Stop();
			return 0; // false will release the ClientSocket
		}

		FString Message = MultiStringFromBinaryArray(ArrayReader);
		
		TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
		FString Endpoint = EndpointAddr->ToString(true);
		BroadcastReceived(Endpoint, Message);

		//FPlatformProcess::Sleep(1.0f);
	}
	//UE_LOG(LogUnrealCV, Log, TEXT(""));
	return 0;
}

void FClientHandler::Stop()
{
	bStop = true;
}

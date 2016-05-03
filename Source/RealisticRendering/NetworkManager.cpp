// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "NetworkManager.h"
#include "Networking.h"
#include <string>

UNetworkManager::UNetworkManager()
{
	ConnectionSocket = NULL;
	Listener = NULL;
	World = NULL;
	WorldTimerManager = NULL;
	bIsConnected = false;
	// TODO: Check unexpected client disconnection
}

UNetworkManager::~UNetworkManager()
{
	// Disconnect current connection and release resource.
	bool Status;
	if (ConnectionSocket)
	{
		Status = ConnectionSocket->Close();
		if (Status)
		{
			UE_LOG(LogTemp, Warning, TEXT("Successfully close connection socket"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Fail to close connection socket"));
		}
	}

	if (Listener)
	{
		Status = Listener->Close();
		if (Status)
		{
			UE_LOG(LogTemp, Warning, TEXT("Successfully close listening socket"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Fail to close listening socket"));
		}
	}
}

void UNetworkManager::ListenSocket()
{
	UE_LOG(LogTemp, Warning, TEXT("Try to start a listening socket"));
	// FString IPAddress = "127.0.0.1";
	// FIPv4Address IPAddress = FIPv4Address(127, 0, 0, 1);
	FIPv4Address IPAddress = FIPv4Address(0, 0, 0, 0);
	uint32 NumPortToTry = 10;
	for (uint32 PortOffset = 0; PortOffset < NumPortToTry; PortOffset++)
	{
		// The configured port might be in use, try a few of them
		uint32 TryPortNum = PortNum + PortOffset;
		FIPv4Endpoint Endpoint(IPAddress, TryPortNum); // TODO: I need to check whether this step is successful.
		// FSocket*
		Listener = FTcpSocketBuilder(TEXT("UnrealCV Controller"))
			.AsReusable().BoundToEndpoint(Endpoint)
			.Listening(1); // Only accept one connection
			// How can I know whether the binding is successful?
			// TODO: Check whether there is any exception thrown in the source code, when the port is in use.
		if (Listener)
		{
			UE_LOG(LogTemp, Warning, TEXT("Start listening on port %d"), TryPortNum);
			break; // The creation is successful.
		}
	}

	if (Listener)
	{
		// When will happen if the endpoint is taken?

		int32 BufferSize = 2 * 1024 * 1024, RealSize;
		Listener->SetReceiveBufferSize(BufferSize, RealSize); // RealSize might be different from what we want

		if (RealSize != BufferSize)
		{
			UE_LOG(LogTemp, Warning, TEXT("The real buffer size %d and expected buffer size %d are different"), RealSize, BufferSize);
		}

		check(Listener); // TODO: Better exception handling
		check(World);

		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, this,
			&UNetworkManager::WaitConnection, 1, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Can not start the listening socket. Unknown error"));
	}
}

void UNetworkManager::WaitConnection()
{
	static int NumAttempt = 0;
	if (!Listener) return; // In case of Socket initialization failed
	// check(Listener);
	NumAttempt += 1;
	if (ConnectionSocket == NULL || ConnectionSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	// I need to check whether current socket is disconnected, if disconnected, start a new connection
	{
		bIsConnected = false;
		UE_LOG(LogTemp, Warning, TEXT("Check connection %d"), NumAttempt);
		TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr(); // Why use TSharedRef? For thread safety?

		bool Pending;
		if (Listener->HasPendingConnection(Pending) && Pending)
		{
			check(ConnectionSocket == NULL); // Assume only one connection
			ConnectionSocket = Listener->Accept(*RemoteAddress, TEXT("Remote socket to send message back"));

			UE_LOG(LogTemp, Warning, TEXT("Client socket connected."));

			if (ConnectionSocket != NULL)
			{
				FTimerHandle TimerHandle;
				World->GetTimerManager().SetTimer(TimerHandle, this, &UNetworkManager::WaitData, 1, true);
				bIsConnected = true;
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Connected"));
	}
}


void UNetworkManager::WaitData()
{
	if (!ConnectionSocket) return;

	TArray<uint8> ReceivedData;

	uint32 Size, MaxSize = 65507u;
	while (ConnectionSocket->HasPendingData(Size))
	{
		// ReceivedData.Init(FMath::Min(Size, MaxSize)); // TODO: Check this magic number
		ReceivedData.SetNumUninitialized(FMath::Min(Size, MaxSize));

		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read); // TODO: Check Robustness.
		// TODO: Define data exchange format? Maybe protobuf?
	}

	if (ReceivedData.Num() <= 0)
	{
		return;
	}
	const FString ReceivedString = StringFromBinaryArray(ReceivedData);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *ReceivedString);
}

FString UNetworkManager::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

void UNetworkManager::SendMessage(FString Message)
{
	// The status check is incorrect. TODO:`
	if (ConnectionSocket == NULL || !bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket is not connected"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Message to client: %s"), *Message);
	TCHAR* SerializedChar = Message.GetCharArray().GetData();
	int32 Size = FCString::Strlen(SerializedChar);
	int32 Sent = 0;
	ConnectionSocket->Send((uint8*)TCHAR_TO_UTF8(SerializedChar), Size, Sent);
}

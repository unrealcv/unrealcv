// Weichao Qiu @ 2016, modified by Hai Ci @ 2022
#pragma once

#include "Runtime/Sockets/Public/Sockets.h"
#include "ClientHandler.h"
#include "Runtime/Core/Public/Serialization/ArrayReader.h"

#if PLATFORM_LINUX
#include <stdlib.h>  
#include <stdio.h>  
#include <stddef.h>  // offsetof()
#include <sys/socket.h> 
#include <sys/un.h>
#include <string.h> // memset()
#include <unistd.h>
#include <ctype.h>
//#include <errno.h>
#endif // PLATFORM_LINUX

//#include "UnrealcvServer.h"

#include "UnixTcpServer.generated.h"


class FUnrealcvServer;

/**
 * a simplified version from FNFSMessageHeader of UnrealEngine4, without CRC check
 */
class FUnixSocketMessageHeader
{
	/** Error checking */
	uint32 Magic = 0;

	/** Payload Size */
	uint32 PayloadSize = 0;

	static uint32 DefaultMagic;
public:
	FUnixSocketMessageHeader(const TArray<uint8>& Payload)
	{
		PayloadSize = Payload.Num();  // What if PayloadSize is 0
		Magic = FUnixSocketMessageHeader::DefaultMagic;
	}

	/** Add header to payload and send it out */
	static bool WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket);
	/** Receive packages and strip header */
	static bool ReceivePayload(FArrayReader& OutPayload, FSocket* Socket);

	// UDS implementation of send and receive data
	/** Add header to payload and send it out */
	static bool WrapAndSendPayloadUDS(const TArray<uint8>& Payload, int fd);
	/** Receive packages and strip header */
	static bool ReceivePayloadUDS(FArrayReader& OutPayload, int fd);
};


// The ; in the end is needed for doxygen.
DECLARE_EVENT_TwoParams(UUnixTcpServer, FReceivedEvent, const FString&, const FString&);
DECLARE_EVENT_OneParam(UUnixTcpServer, FErrorEvent, const FString&);
DECLARE_EVENT_OneParam(UUnixTcpServer, FConnectedEvent, const FString&);

/**
 * Server to send and receive message
 */
UCLASS()
class UNREALCV_API UUnixTcpServer : public UObject
// TcpServer needs to be an UObject, so that we can bind ip and port to UI.
{
	GENERATED_BODY()

public:
	/** The port number this server is listening on */
	int32 PortNum = -1;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	// bool bIsConnected = false;
	bool IsConnected();

	bool IsListening()
	{
		return bIsListening;
	}

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	// FString ListenIP = "0.0.0.0"; // TODO: this is hard coded right now

	/** Start the underlying TcpListener to listen for new connection */
	// TODO: Handle port in use exception
	bool Start(int32 InPortNum);

	/** Send a string to connected client, return false if false to send. Will fail if no connection available */
	bool SendMessage(const FString& Message);

	/** Send a byte array to connected client, return false if failed to send. */
	bool SendData(const TArray<uint8>& Payload);

	/** Send a string to connected client, return false if false to send. Will fail if no connection available */
	bool SendMessageINet(const FString& Message);

	/** Send a byte array to connected client, return false if failed to send. */
	bool SendDataINet(const TArray<uint8>& Payload);

	/** Send a string with UDS, only works on Linux */
	bool SendMessageUDS(const FString& Message);

	/** Send a byte array with UDS, only works on Linux */
	bool SendDataUDS(const TArray<uint8>& Payload);

	FReceivedEvent& OnReceived() { return ReceivedEvent;  } // The reference can not be changed

	FErrorEvent& OnError() { return ErrorEvent;  } // The reference can not be changed

	TArray<FClientHandler*> RunnerClients;

	FUnrealcvServer* UnrealcvServer;

private:
	/** Is the listening socket running */
	bool bIsListening = false;

	/** Handle a new connected client, need to decide accept of reject */
	bool Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/** Handle multiple connections to the server */
	bool Multi_Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/** The connected client socket, only maintain one client at a time */
	FSocket* ConnectionSocket; // FSimpleAbstractSocket's receive is hard to use for non-blocking mode

	// UDS socket file descriptor
	int UDS_connfd = -1;
	int UDS_listenfd = -1;  // used to shutdown blocking accept()

	// Switch flag bewteen TCP and UDS
	bool bIsUDS = false;

	/** TcpListener used to listen new incoming connection */
	TSharedPtr<FTcpListener> TcpListener;

	~UUnixTcpServer();

	/** (Debug) Start a service that echo whatever it got, for debug purpose */
	bool StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/** Start a service to handle incoming message, ReceivedEvent will be fired when a new message arrive */
	bool StartMessageServiceINet(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/** Start a service to handle incoming message from multiple cilents*/
	bool StartMessageServiceINet_Multi(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint);

	/** Start a service to handle incoming message, with UDS, blocking service*/
	bool StartMessageServiceUDS();

	/** Event handler for event `Received` */
	FReceivedEvent ReceivedEvent;

	/** Event handler for event `Error` */
	FErrorEvent ErrorEvent;

	/** Event handler for event `Connected` */
	FConnectedEvent ConnectedEvent;

	/** Broadcast event `Received` */
	void BroadcastReceived(const FString& Endpoint, const FString& Message)
	{
		ReceivedEvent.Broadcast(Endpoint, Message);
	}

	/** Broadcast event `Connected` */
	void BroadcastConnected(const FString& Message)
	{
		ConnectedEvent.Broadcast(Message);
	}
};
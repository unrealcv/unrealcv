// Weichao Qiu @ 2016, modified by Hai Ci @ 2022
#include "UnixTcpServer.h"
#include "Runtime/Core/Public/Serialization/BufferArchive.h"
#include "Runtime/Core/Public/Serialization/MemoryReader.h"
#include <string>
#include "UnrealcvShim.h"

#include "ConsoleHelper.h"
#include "Commands/ObjectHandler.h"
#include "Commands/PluginHandler.h"
#include "Commands/ActionHandler.h"
#include "Commands/AliasHandler.h"
#include "Commands/CameraHandler.h"
#include "WorldController.h"
#include "UnrealcvLog.h"

#include "UnrealcvServer.h"

uint32 FUnixSocketMessageHeader::DefaultMagic = 0x9E2B83C1;

bool FUnixSocketMessageHeader::WrapAndSendPayload(const TArray<uint8>& Payload, FSocket* Socket)
{
	FUnixSocketMessageHeader Header(Payload);

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
bool UnixSocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize)
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

bool FUnixSocketMessageHeader::ReceivePayload(FArrayReader& OutPayload, FSocket* Socket)
{
	if (Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Trying to read message from an unconnected socket."));
	}
	TArray<uint8> HeaderBytes;
	int32 Size = sizeof(FUnixSocketMessageHeader);
	HeaderBytes.AddZeroed(Size);

	if (!UnixSocketReceiveAll(Socket, HeaderBytes.GetData(), Size))
	{
		// false here means socket disconnected.
		// UE_LOG(LogUnrealCV, Error, TEXT("Unable to read header, Socket disconnected."));
		UE_LOG(LogUnrealCV, Log, TEXT("Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic;
	Reader << Magic;

	if (Magic != FUnixSocketMessageHeader::DefaultMagic)
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
	if (!UnixSocketReceiveAll(Socket, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unable to read full payload, Socket disconnected."));
		return false;
	}

	// Skip CRC checking in FNFSMessageHeader
	return true;
}


bool FUnixSocketMessageHeader::WrapAndSendPayloadUDS(const TArray<uint8>& Payload, int fd)
{
	#if PLATFORM_LINUX
	FUnixSocketMessageHeader Header(Payload);

	FBufferArchive Ar;
	Ar << Header.Magic;
	Ar << Header.PayloadSize;
	Ar.Append(Payload);

	int32 AmountAlreadySent = 0; // How many bytes have been sent
	int32 AmountToSend = Ar.Num();
	int AmountActuallySent = 0;
	int NumTrial = 1000; // Only try a limited amount of times
	while (AmountToSend > 0)
	{
		
		// GetData returns a uint8 pointer
		AmountActuallySent = write(fd, Ar.GetData() + AmountAlreadySent, AmountToSend);

		//Socket->Send(Ar.GetData() + TotalAmountSent, Ar.Num() - TotalAmountSent, AmountSent);
		NumTrial--;

		if (AmountActuallySent == -1)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Unsuccessful send %hs"), strerror(errno));
			close(fd);
			return false;
		}
		if (NumTrial < 0)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Unable to send(try 1000 times). Expect to send %d, sent %d"), Ar.Num(), AmountAlreadySent);
			close(fd);
			return false;
		}

		UE_LOG(LogUnrealCV, Verbose, TEXT("Already sent bytes %d/%d, new send %d"), AmountAlreadySent, Ar.Num(), AmountActuallySent);
		AmountToSend -= AmountActuallySent;
		AmountAlreadySent += AmountActuallySent;
	}
	check(AmountToSend == 0);
	#endif // PLATFORM_LINUX
	return true;
}

/* Waiting for data, return false only when disconnected */
bool UnixSocketReceiveAllUDS(int fd, uint8* Result, int32 ExpectedSize)
{
	// Make sure no error before using this socket
	// ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
	// const TCHAR* LastErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
	// UE_LOG(LogUnrealCV, Display, TEXT("Error msg before receiving data %s"), LastErrorMsg);

	#if PLATFORM_LINUX
	int32 Offset = 0;
	int32 NumRead = 0;
	while (ExpectedSize > 0)
	{
		// uint32 PendingDataSize;
		// bool Status = Socket->HasPendingData(PendingDataSize);
		// status = PendingDataSize != 0
		NumRead = read(fd, Result + Offset, ExpectedSize);
		//bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead);
		
		// bool RecvStatus = Socket->Recv(Result + Offset, ExpectedSize, NumRead, ESocketReceiveFlags::WaitAll);
		// WaitAll is not effective for non-blocking socket, see here https://msdn.microsoft.com/en-us/library/windows/desktop/ms740121(v=vs.85).aspx
		// Check pending data first, see https://msdn.microsoft.com/en-us/library/windows/desktop/ms738573(v=vs.85).aspx

		// ESocketConnectionState ConnectionState = Socket->GetConnectionState();
		// check(NumRead <= ExpectedSize);
		// RecvStatus == BytesRead >= 0
		check(NumRead <= ExpectedSize);

		if (NumRead > 0) // successfully read something
		{
			// Got some data and in an expected condition
			Offset += NumRead;
			ExpectedSize -= NumRead;
			continue;
		}
		else if (NumRead == 0)
		{
			
			UE_LOG(LogUnrealCV, Log, TEXT("The connection is gracefully closed by the client."));
			close(fd);
			return false; // Socket is disconnected.
		}
		else
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Server socket failed to read: %hs"), strerror(errno));
			close(fd);
			return false;

			//if (LastError == ESocketErrors::SE_EWOULDBLOCK)
			//{
			//	continue; // No data and keep waiting
			//}
			//// Use this check instead of use return status of recv to ensure backward compatibility
			//// Because there is a bug with 4.12 FSocketBSD implementation
			//// https://www.unrealengine.com/blog/unreal-engine-4-13-released, Search FSocketBSD::Recv
			//if (LastError == ESocketErrors::SE_NO_ERROR) // 0 means gracefully closed
			//{
			//	UE_LOG(LogUnrealCV, Log, TEXT("The connection is gracefully closed by the client."));
			//	return false; // Socket is disconnected. if -1, keep waiting for data
			//}

			//// LastError == ESocketErrors::SE_EWOULDBLOCK means running a non-block socket."));
			//if (LastError == ESocketErrors::SE_ECONNABORTED) // SE_ECONNABORTED
			//{
			//	UE_LOG(LogUnrealCV, Error, TEXT("Connection aborted unexpectly."));
			//	return false;
			//}

			//if (LastError == ESocketErrors::SE_ENOTCONN)
			//{
			//	UE_LOG(LogUnrealCV, Error, TEXT("Socket is not connected."));
			//	return false;
			//}

			/*const TCHAR* LastErrorMsg = ISocketSubsystem::Get()->GetSocketError(LastError);
			UE_LOG(LogUnrealCV, Error, TEXT("Unexpected error of socket happend, error %s"), LastErrorMsg);

			return false;*/
		}
	}
	#endif // PLATFORM_LINUX
	return true;
}

bool FUnixSocketMessageHeader::ReceivePayloadUDS(FArrayReader& OutPayload, int fd)
{
	/*if (Socket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Trying to read message from an unconnected socket."));
	}*/
	// TODO: log connected status


	TArray<uint8> HeaderBytes;
	int32 Size = sizeof(FUnixSocketMessageHeader);
	HeaderBytes.AddZeroed(Size);

	if (!UnixSocketReceiveAllUDS(fd, HeaderBytes.GetData(), Size))
	{
		// false here means socket disconnected.
		// UE_LOG(LogUnrealCV, Error, TEXT("Unable to read header, Socket disconnected."));
		UE_LOG(LogUnrealCV, Log, TEXT("Unable to read header, Client disconnected."));
		return false;
	}

	FMemoryReader Reader(HeaderBytes);
	uint32 Magic;
	Reader << Magic;

	if (Magic != FUnixSocketMessageHeader::DefaultMagic)
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
	if (!UnixSocketReceiveAllUDS(fd, OutPayload.GetData() + PayloadOffset, PayloadSize))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unable to read full payload, Socket disconnected."));
		return false;
	}

	// Skip CRC checking in FNFSMessageHeader
	return true;
}

FString UnixStringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

void UnixBinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	//From: https://github.com/EpicGames/UnrealEngine/blob/5.3/Engine/Source/Runtime/Core/Public/Containers/StringConv.h#L339
	/*UE_DEPRECATED(5.1, "FTCHARToUTF8_Convert has been deprecated in favor of FPlatformString::Convert and StringCast")*/
#if ENGINE_MAJOR_VERSION <= 4
	FTCHARToUTF8 Convert(*Message);

	OutBinaryArray.Empty();

	// const TArray<TCHAR>& CharArray = Message.GetCharArray();
	// OutBinaryArray.Append(CharArray);
	// This can work, but will add tailing \0 also behavior is not well defined.

	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
#else 
	//https://github.com/EpicGames/UnrealEngine/blob/5.3/Engine/Source/Runtime/Core/Public/Containers/StringConv.hL#L1070
	auto converter = StringCast<UTF8CHAR>(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((uint8*)converter.Get(), converter.Length());
#endif
}


bool UUnixTcpServer::IsConnected()
{
	return (this->ConnectionSocket != nullptr);
}

/* Provide a dummy echo service to echo received data back for development purpose */
bool UUnixTcpServer::StartEchoService(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
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
		while (true)
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

// not a member function
bool StartUDSMessageService_test()
{
	#if PLATFORM_LINUX
	char const *socket_path = "server.socket";
	// setbuf(stdout, NULL);  // for intermediate print
	struct sockaddr_un serun, cliun;
	socklen_t cliun_len;
	int listenfd, connfd, size;
	char buf[80];
	UE_LOG(LogUnrealCV, Log, TEXT("sizeof buf: %lu\n"), sizeof(buf));
	//printf("sizeof buf: %lu\n", sizeof(buf));

	int i, n;
	int close_flag = 0;

	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		//perror("socket error");
		UE_LOG(LogUnrealCV, Error, TEXT("socket error"));
		return false;
	}

	memset(&serun, 0, sizeof(serun));
	serun.sun_family = AF_UNIX;
	strcpy(serun.sun_path, socket_path);
	size = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);
	unlink(socket_path);
	if (bind(listenfd, (struct sockaddr*)&serun, size) < 0) {
		//perror("bind error");
		UE_LOG(LogUnrealCV, Error, TEXT("bind error"));
		//exit(1);
		return false;
	}
	printf("UNIX domain socket bound\n");

	if (listen(listenfd, 20) < 0) {
		//perror("listen error");
		UE_LOG(LogUnrealCV, Error, TEXT("listen error"));
		//exit(1);
		return false;
	}
	printf("Accepting connections ...\n");

	for (i = 0; i < 2; i++) {
		cliun_len = sizeof(cliun);
		// block at accpet()
		if ((connfd = accept(listenfd, (struct sockaddr*)&cliun, &cliun_len)) < 0) {
			//perror("accept error");
			UE_LOG(LogUnrealCV, Error, TEXT("accept error"));
			continue;
		}

		//printf("new connection established !\n");
		UE_LOG(LogUnrealCV, Warning, TEXT("new connection established !"));

		while (1) {
			// memset(buf, 0, MAXLINE);
			n = read(connfd, buf, sizeof(buf));  // block here
			if (n < 0) {
				UE_LOG(LogUnrealCV, Error, TEXT("read error"));
				//perror("read error\n");
				break;
			}
			else if (n == 0) {
				//printf("EOF\n");
				UE_LOG(LogUnrealCV, Warning, TEXT("EOF"));
				close_flag = 1;
				break;
			}

			// if (buf[n-1] == '\n') {
			//     printf("you are right !");
			// }
			UE_LOG(LogUnrealCV, Warning, TEXT("received something"));
			//printf("received: %.*s", n, buf);  // print first n of buf
			// fflush(stdout);

			/*for (i = 0; i < n; i++) {
				buf[i] = toupper(buf[i]);
			}*/

			TArray<uint8> Payload;
			FString Message = FString(TEXT("connected to UDS server\n"));
			UnixBinaryArrayFromString(Message, Payload);
			int32 write_n = write(connfd, Payload.GetData(), Payload.Num());
			UE_LOG(LogUnrealCV, Warning, TEXT("%d bytes writes !"), write_n);

			//write(connfd, buf, n);
		}
		close(connfd);
		UE_LOG(LogUnrealCV, Warning, TEXT("this connection exit, waiting for new connections !"));
		//printf("this connection exit, waiting for new connections !\n");
		// if (close_flag) {
		//     break;
		// }
	}
	close(listenfd);
	//printf("server quit !\n");
	UE_LOG(LogUnrealCV, Warning, TEXT("server quit"));
	#endif // PLATFORM_LINUX
	return false;
}


bool UUnixTcpServer::StartMessageServiceUDS()
{
#if PLATFORM_LINUX
	if (UDS_connfd != -1)
	{
		// No response and let the client silently timeout
		UE_LOG(LogUnrealCV, Warning, TEXT("Only one client is allowed, can not allow new connection"));
		return false; // Already have a connection
	}

	std::string s = "/tmp/unrealcv_";
	s += std::to_string(PortNum);
	s += ".socket";
	char const* socket_path = s.c_str();
	UE_LOG(LogUnrealCV, Log, TEXT("uds server socket created at: %s"), *FString(s.c_str()));

	//char const* socket_path = "server.socket";
	// setbuf(stdout, NULL);  // for intermediate print
	struct sockaddr_un serun, cliun;
	socklen_t cliun_len;
	int listenfd, connfd, size;

	/*char buf[80];
	UE_LOG(LogUnrealCV, Log, TEXT("sizeof buf: %lu\n"), sizeof(buf));*/
	UE_LOG(LogUnrealCV, Log, TEXT("start UDS service"));

	int i, n;

	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		UE_LOG(LogUnrealCV, Error, TEXT("socket error"));
		return false;
	}

	memset(&serun, 0, sizeof(serun));
	serun.sun_family = AF_UNIX;
	strcpy(serun.sun_path, socket_path);
	size = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);
	unlink(socket_path);
	if (bind(listenfd, (struct sockaddr*)&serun, size) < 0) {
		UE_LOG(LogUnrealCV, Error, TEXT("bind error"));
		close(listenfd);
		return false;
	}

	UE_LOG(LogUnrealCV, Log, TEXT("UNIX domain socket bound..."));

	if (listen(listenfd, 5) < 0) {
		// listen(5) actually accepts much more than 5 connection requests
		UE_LOG(LogUnrealCV, Error, TEXT("listen error"));
		close(listenfd);
		return false;
	}
	//set listening socket handler
	UDS_listenfd = listenfd;
	
	while (1) {
		UE_LOG(LogUnrealCV, Log, TEXT("Accepting connections ..."));
		// keep accepting new connections
		cliun_len = sizeof(cliun);
		// block at accpet()
		if ((connfd = accept(UDS_listenfd, (struct sockaddr*)&cliun, &cliun_len)) < 0) {
			// if shutdown() is called, then break and exit
			UE_LOG(LogUnrealCV, Error, TEXT("accept error"));
			break;
		}
		UE_LOG(LogUnrealCV, Warning, TEXT("new connection established !"));

		// set connection handler for this class
		UDS_connfd = connfd;

		// This message is necessary for client to confirm successful connection
		FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
		UE_LOG(LogUnrealCV, Log, TEXT("sending confirm message"));
		bool IsSent = SendMessageUDS(Confirm); // Send a hello message through UDS IPC
		if (!IsSent)
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to send welcome message to client"));
			UDS_connfd = -1;
		}

		while (UDS_connfd != -1) {
			// while this connection is active

			FArrayReader ArrayReader;
			if (!FUnixSocketMessageHeader::ReceivePayloadUDS(ArrayReader, UDS_connfd))
				// Wait forever until got a message, or return false when error happened
			{
				// FIX: important to close the connection, otherwise listening socket may refuse new connection
				// https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
				//this->ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
				//this->ConnectionSocket->Close();
				//this->ConnectionSocket = NULL;

				//close(UDS_connfd);  // close in the underlying function
				UDS_connfd = -1;
				break;
			}

			FString Message = UnixStringFromBinaryArray(ArrayReader);

			/*TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
			ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
			FString Endpoint = EndpointAddr->ToString(true);*/
			
			// create a fake Endpoint
			FString Endpoint = FString(TEXT("UDS client"));
			BroadcastReceived(Endpoint, Message);

			/*TArray<uint8> Payload;
			FString Message = FString(TEXT("connected to UDS server\n"));
			UnixBinaryArrayFromString(Message, Payload);
			int32 write_n = write(connfd, Payload.GetData(), Payload.Num());
			UE_LOG(LogUnrealCV, Warning, TEXT("%d bytes writes !"), write_n);*/
		}
		//close(connfd);  //close connfd in the underlying function
		UE_LOG(LogUnrealCV, Warning, TEXT("this connection exit, waiting for new connections !"));
	}
	close(UDS_listenfd);
	UDS_listenfd = -1;
	UDS_connfd = -1;
	UE_LOG(LogUnrealCV, Warning, TEXT("UDS server quit"));
#endif // PLATFORM_LINUX
	return false;
}


/**
  * Start message service in listening thread
  * TODO: Start a new background thread to receive message
  */
bool UUnixTcpServer::StartMessageServiceINet(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	if (this->ConnectionSocket)
	{
		// No response and let the client silently timeout
		UE_LOG(LogUnrealCV, Warning, TEXT("Only one client is allowed, can not allow new connection from %s"), *ClientEndpoint.ToString());
		return false; // Already have a connection
	}

	ConnectionSocket = ClientSocket;

	UE_LOG(LogUnrealCV, Warning, TEXT("New client connected from %s"), *ClientEndpoint.ToString());
	// ClientSocket->SetNonBlocking(false); // When this in blocking state, I can not use this socket to send message back
	FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
	bool IsSent = this->SendMessageINet(Confirm); // Send a hello message through IP-port 
	if (!IsSent)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to send welcome message to client."));
	}

	// TODO: Start a new thread
	while (ConnectionSocket) // Listening thread, while the client is still connected
	{
		if (ConnectionSocket->GetConnectionState() != ESocketConnectionState::SCS_Connected)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Trying to read message from an unconnected socket."));
		}

		FArrayReader ArrayReader;
		if (!FUnixSocketMessageHeader::ReceivePayload(ArrayReader, ConnectionSocket))
			// Wait forever until got a message, or return false when error happened
		{
			// FIX: important to close the connection, otherwise listening socket may refuse new connection
			// https://stackoverflow.com/questions/4160347/close-vs-shutdown-socket
			this->ConnectionSocket->Shutdown(ESocketShutdownMode::ReadWrite);
			this->ConnectionSocket->Close();
			this->ConnectionSocket = NULL;
			return false; // false will release the ClientSocket
		}

		FString Message = UnixStringFromBinaryArray(ArrayReader);

		TSharedRef<FInternetAddr> EndpointAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
		ConnectionSocket->GetPeerAddress(EndpointAddr.Get());
		FString Endpoint = EndpointAddr->ToString(true);
		BroadcastReceived(Endpoint, Message);

		// Fire raw message received event, use message id to connect request and response
		// UE_LOG(LogUnrealCV, Warning, TEXT("Receive message %s"), *Message);
	}
	return false; // TODO: What is the meaning of return value?
}

bool UUnixTcpServer::StartMessageServiceINet_Multi(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	return false;
}

/** Connected Handler */
bool UUnixTcpServer::Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	// When a new connecton conneted to the server, there's a while loop in the StartMessageServiceINet will be executed permenantely which will prevent the server from doing anything else.
	bool ServiceStatus = true;
	BroadcastConnected(*ClientEndpoint.ToString());
	// ServiceStatus = StartEchoService(ClientSocket, ClientEndpoint);
	ServiceStatus = StartMessageServiceINet(ClientSocket, ClientEndpoint);
	bIsUDS = true;
	ServiceStatus = StartMessageServiceUDS();
	return ServiceStatus;
	// This is a blocking service, if need to support multiple connections, consider start a new thread here.
}


bool UUnixTcpServer::Multi_Connected(FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	bool ServiceStatus = true;

	//{
	//	// Lock thread and add a new client to the server
	//	FScopeLock Lock(&SocketLock);
	//	ClientSockets.Add(ClientSocket);
	//}
	UE_LOG(LogUnrealCV, Warning, TEXT("BroadCast the connection (multi_connect)"));

	BroadcastConnected(*ClientEndpoint.ToString());
	ConnectionSocket = ClientSocket;
	FString Confirm = FString::Printf(TEXT("connected to %s"), *GetProjectName());
	bool IsSent = this->SendMessageINet(Confirm); // Send a hello message through IP-port 
	if (!IsSent)
	{ 
		UE_LOG(LogUnrealCV, Error, TEXT("Failed to send welcome message to client."));
	}

	
	FClientHandler* RunnerClient = new FClientHandler(ClientSocket, ClientEndpoint);

	RunnerClients.Add(RunnerClient);

	FRunnableThread *thread = RunnerClient->Thread;

	// Log the status of the threads
	UE_LOG(LogUnrealCV, Warning, TEXT("Current number of threads: %d"), RunnerClients.Num());
	for (FClientHandler* client : RunnerClients)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Thread ID: %d"), client->Thread->GetThreadID());
	}

	// bind OnReceived of FClientHandler to BroadcastReceived of UUnixTcpServer 
	RunnerClient->OnReceived().AddLambda([this](const FString& Endpoint, const FString& Message) {
		this->BroadcastReceived(Endpoint, Message);
		this->UnrealcvServer->HandleRawMessage(Endpoint, Message); // entry function that exec the commands
	});
	return ServiceStatus;
}


bool UUnixTcpServer::Start(int32 InPortNum) // Restart the server if configuration changed
{
	UE_LOG(LogUnrealCV, Warning, TEXT("(UnixTcpServer) Start listening on port %d"), InPortNum);
	if (InPortNum == this->PortNum && this->bIsListening) return true; // Already started

	if (ConnectionSocket) // Release previous connection, one game only check once because multi-connection is handled by client handler
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Release previous connection"));
		ConnectionSocket->Close();
		ConnectionSocket = NULL;
	}

	if (TcpListener.IsValid()) // Delete previous configuration first
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Stop previous server"));
		TcpListener->Stop(); // TODO: test the robustness, will this operation successful?
	}

	this->PortNum = InPortNum; // Start a new TCPListener
	FIPv4Address IPAddress = FIPv4Address(0, 0, 0, 0);
	// int32 PortNum = this->PortNum; // Make this configuable
	FIPv4Endpoint Endpoint(IPAddress, PortNum);

	int MaxConnection = 1;
	FSocket* ServerSocket = FTcpSocketBuilder(TEXT("FTcpListener server")) // TODO: Need to realease this socket
		// .AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(MaxConnection);

	if (ServerSocket)
	{
		int32 NewSize = 0;
		ServerSocket->SetReceiveBufferSize(2 * 1024 * 1024, NewSize);
	}
	else
	{
		this->bIsListening = false;
		UE_LOG(LogUnrealCV, Warning, TEXT("Cannot start listening on port %d, Port might be in use"), PortNum);
		// This message can not be error. Error will prevent cook server from launching.
		return false;
	}

	TcpListener = TSharedPtr<FTcpListener>(new FTcpListener(*ServerSocket));
	// TcpListener = new FTcpListener(Endpoint); // This will be released after start
	// In FSocket, when a FSocket is set as reusable, it means SO_REUSEADDR, not SO_REUSEPORT.  see SocketsBSD.cpp
	TcpListener->OnConnectionAccepted().BindUObject(this, &UUnixTcpServer::Multi_Connected); // bind connected event
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


bool UUnixTcpServer::SendMessage(const FString& Message)
{
	// send confirm message; and send blueprint message
	#if PLATFORM_LINUX
	if (bIsUDS)
	{
		return SendMessageUDS(Message);
	}
	else
	{
		return SendMessageINet(Message);
	}
	#else
	return SendMessageINet(Message);
	#endif
}

bool UUnixTcpServer::SendData(const TArray<uint8>& Payload)
{
#if PLATFORM_LINUX
	if (bIsUDS)
	{
		return SendDataUDS(Payload);
	}
	else
	{
		return SendDataINet(Payload);
	}
#else
	return SendDataINet(Payload);
#endif
}

bool UUnixTcpServer::SendMessageINet(const FString& Message)
{
	if (ConnectionSocket)
	{
		TArray<uint8> Payload;
		UnixBinaryArrayFromString(Message, Payload);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send string message with size %d"), Payload.Num());
		FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"), Payload.Num());
		return true;
	}
	return false;
}

bool UUnixTcpServer::SendDataINet(const TArray<uint8>& Payload)
{
	if (ConnectionSocket)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send binary payload with size %d"), Payload.Num());
		FUnixSocketMessageHeader::WrapAndSendPayload(Payload, ConnectionSocket);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"), Payload.Num());
		return true;
	}
	return false;
}

bool UUnixTcpServer::SendMessageUDS(const FString& Message)
{
	if (UDS_connfd != -1)
	{
		TArray<uint8> Payload;
		UnixBinaryArrayFromString(Message, Payload);
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send string message with size %d"), Payload.Num());
		if (FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_connfd))
		{
			UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"));
			return true;
		}
		else
		{
			UE_LOG(LogUnrealCV, Verbose, TEXT("Error during Payload sending"));
			return false;
		}
	}
	UE_LOG(LogUnrealCV, Error, TEXT("UDS file descriptor is invalid"));
	return false;
}

bool UUnixTcpServer::SendDataUDS(const TArray<uint8>& Payload)
{
	// The interface between UnrealcvServer and TCPServer
	// Send data get from unreal engine to the client who requests.
	if (UDS_connfd != -1)
	{
		UE_LOG(LogUnrealCV, Verbose, TEXT("Send binary payload with size %d"), Payload.Num());
		if (FUnixSocketMessageHeader::WrapAndSendPayloadUDS(Payload, UDS_connfd))
		{
			UE_LOG(LogUnrealCV, Verbose, TEXT("Payload sent"));
			return true;
		}
		else
		{
			UE_LOG(LogUnrealCV, Verbose, TEXT("Error during Payload sending"));
			return false;
		}
	}

	// TODO: exist if fd == -1?
	UE_LOG(LogUnrealCV, Error, TEXT("UDS file descriptor is invalid"));
	return false;
}

UUnixTcpServer::~UUnixTcpServer()
{
	#if PLATFORM_LINUX
	if (UDS_connfd != -1)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Destroy connection socket"));
		shutdown(UDS_connfd, SHUT_RDWR);
		close(UDS_connfd);
		UDS_connfd = -1;
	}
	if (UDS_listenfd != -1)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Destroy listening socket"));
		shutdown(UDS_listenfd, SHUT_RDWR);
		close(UDS_listenfd);
		UDS_listenfd = -1;
	}
	#endif
}

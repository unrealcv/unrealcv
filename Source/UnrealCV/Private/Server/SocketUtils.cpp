// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "SocketUtils.h"
#include "UnrealcvLog.h"
#include <string>

#if PLATFORM_LINUX
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif

namespace UCV::SocketUtils
{

FString StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	const std::string Utf8(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(UTF8_TO_TCHAR(Utf8.c_str()));
}

void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	const auto Converter = StringCast<UTF8CHAR>(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
}

bool SocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize)
{
	check(Socket != nullptr);
	int32 Offset = 0;
	while (ExpectedSize > 0)
	{
		int32 NumRead = 0;
		Socket->Recv(Result + Offset, ExpectedSize, NumRead);

		if (NumRead > 0)
		{
			check(NumRead <= ExpectedSize);
			Offset       += NumRead;
			ExpectedSize -= NumRead;
			continue;
		}

		const ESocketErrors LastError = ISocketSubsystem::Get()->GetLastErrorCode();
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
		if (NumRead > 0)
		{
			check(NumRead <= ExpectedSize);
			Offset       += NumRead;
			ExpectedSize -= NumRead;
			continue;
		}
		if (NumRead == 0)
		{
			UE_LOG(LogUnrealCV, Log, TEXT("UDS connection gracefully closed."));
			return false;
		}
		UE_LOG(LogUnrealCV, Error, TEXT("UDS read error: %hs"), strerror(errno));
		return false;
	}
	return true;
}
#endif

} // namespace UCV::SocketUtils

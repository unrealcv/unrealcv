// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Sockets/Public/Sockets.h"

/**
 * Common socket I/O utilities shared between UTcpServer and UUnixTcpServer.
 *
 * Placed in a namespace to avoid ODR issues and keep free functions discoverable.
 */
namespace UCV::SocketUtils
{

/** Convert a binary payload to an FString via UTF-8. */
[[nodiscard]] FString StringFromBinaryArray(const TArray<uint8>& BinaryArray);

/** Convert an FString to a UTF-8 binary payload. */
void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray);

/**
 * Blocking receive of exactly @p ExpectedSize bytes.
 *
 * @return true if all bytes were received; false on disconnect / error.
 */
[[nodiscard]] bool SocketReceiveAll(FSocket* Socket, uint8* Result, int32 ExpectedSize);

#if PLATFORM_LINUX
/**
 * Blocking receive of exactly @p ExpectedSize bytes over a Unix domain socket.
 *
 * @return true if all bytes were received; false on disconnect / error.
 */
[[nodiscard]] bool UDSReceiveAll(int Fd, uint8* Result, int32 ExpectedSize);
#endif

} // namespace UCV::SocketUtils

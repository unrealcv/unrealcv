#include "Utils/SharedMemoryManager.h"

#include "HAL/PlatformProcess.h"
#include "UnrealcvLog.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/WindowsHWrapper.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif

struct FUnrealCVSharedMemoryManager::FSharedMemoryRegion
{
    FString Name;
    int64 NumBytes = 0;
    uint64 Version = 0;
    void* Data = nullptr;

#if PLATFORM_WINDOWS
    HANDLE MappingHandle = nullptr;
#endif

    ~FSharedMemoryRegion()
    {
#if PLATFORM_WINDOWS
        if (Data)
        {
            UnmapViewOfFile(Data);
            Data = nullptr;
        }
        if (MappingHandle)
        {
            CloseHandle(MappingHandle);
            MappingHandle = nullptr;
        }
#endif
    }
};

FUnrealCVSharedMemoryManager& FUnrealCVSharedMemoryManager::Get()
{
    static FUnrealCVSharedMemoryManager Instance;
    return Instance;
}

bool FUnrealCVSharedMemoryManager::WriteBytes(const FString& LogicalName, const void* SourceData, int64 NumBytes,
                                              FUnrealCVSharedMemoryView& OutView, FString& OutError)
{
    const double StartTime = FPlatformTime::Seconds();
    if (!SourceData)
    {
        OutError = TEXT("Source data is null");
        return false;
    }
    if (NumBytes <= 0)
    {
        OutError = TEXT("Shared memory size must be positive");
        return false;
    }

    FScopeLock Lock(&CriticalSection);
    const double MapStartTime = FPlatformTime::Seconds();
    if (!CreateOrResizeRegion(LogicalName, NumBytes, OutView, OutError))
    {
        return false;
    }
    const double MapEndTime = FPlatformTime::Seconds();

    const double MemcpyStartTime = FPlatformTime::Seconds();
    FMemory::Memcpy(OutView.Data, SourceData, NumBytes);
    const double EndTime = FPlatformTime::Seconds();
    OutView.MapMs = (MapEndTime - MapStartTime) * 1000.0;
    OutView.MemcpyMs = (EndTime - MemcpyStartTime) * 1000.0;
    OutView.TotalMs = (EndTime - StartTime) * 1000.0;
    UE_LOG(LogUnrealCV, Warning,
           TEXT("PERFTRACE|shared_memory_write|logical=%s|bytes=%lld|map_ms=%.3f|memcpy_ms=%.3f|total_ms=%.3f"),
           *LogicalName, NumBytes, OutView.MapMs, OutView.MemcpyMs, OutView.TotalMs);
    return true;
}

bool FUnrealCVSharedMemoryManager::Release(const FString& LogicalName)
{
    FScopeLock Lock(&CriticalSection);
    return Regions.Remove(LogicalName) > 0;
}

FString FUnrealCVSharedMemoryManager::MakePlatformName(const FString& LogicalName, uint64 Version) const
{
    FString Sanitized;
    Sanitized.Reserve(LogicalName.Len());
    for (const TCHAR Char : LogicalName)
    {
        if (FChar::IsAlnum(Char) || Char == TEXT('_') || Char == TEXT('-'))
        {
            Sanitized.AppendChar(Char);
        }
        else
        {
            Sanitized.AppendChar(TEXT('_'));
        }
    }
    return FString::Printf(TEXT("UnrealCV_%u_%s_v%llu"), FPlatformProcess::GetCurrentProcessId(), *Sanitized, Version);
}

bool FUnrealCVSharedMemoryManager::CreateOrResizeRegion(const FString& LogicalName, int64 NumBytes,
                                                        FUnrealCVSharedMemoryView& OutView, FString& OutError)
{
#if PLATFORM_WINDOWS
    TUniquePtr<FSharedMemoryRegion>* ExistingRegion = Regions.Find(LogicalName);
    if (ExistingRegion && ExistingRegion->IsValid() && (*ExistingRegion)->NumBytes >= NumBytes &&
        (*ExistingRegion)->Data)
    {
        OutView.Name = (*ExistingRegion)->Name;
        OutView.NumBytes = NumBytes;
        OutView.Version = (*ExistingRegion)->Version;
        OutView.OffsetBytes = 0;
        OutView.Data = (*ExistingRegion)->Data;
        return true;
    }

    Regions.Remove(LogicalName);

    uint64& Version = RegionVersions.FindOrAdd(LogicalName);
    Version += 1;
    const FString PlatformName = MakePlatformName(LogicalName, Version);
    const uint64 Size = static_cast<uint64>(NumBytes);
    const DWORD SizeHigh = static_cast<DWORD>((Size >> 32) & 0xffffffff);
    const DWORD SizeLow = static_cast<DWORD>(Size & 0xffffffff);

    HANDLE MappingHandle =
        CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, SizeHigh, SizeLow, *PlatformName);
    if (!MappingHandle)
    {
        OutError =
            FString::Printf(TEXT("CreateFileMappingW failed for '%s' with error %lu"), *PlatformName, GetLastError());
        return false;
    }

    void* MappedData = MapViewOfFile(MappingHandle, FILE_MAP_ALL_ACCESS, 0, 0, static_cast<SIZE_T>(NumBytes));
    if (!MappedData)
    {
        const DWORD ErrorCode = GetLastError();
        CloseHandle(MappingHandle);
        OutError = FString::Printf(TEXT("MapViewOfFile failed for '%s' with error %lu"), *PlatformName, ErrorCode);
        return false;
    }

    TUniquePtr<FSharedMemoryRegion> Region = MakeUnique<FSharedMemoryRegion>();
    Region->Name = PlatformName;
    Region->NumBytes = NumBytes;
    Region->Version = Version;
    Region->Data = MappedData;
    Region->MappingHandle = MappingHandle;

    OutView.Name = Region->Name;
    OutView.NumBytes = NumBytes;
    OutView.Version = Region->Version;
    OutView.OffsetBytes = 0;
    OutView.Data = Region->Data;

    Regions.Add(LogicalName, MoveTemp(Region));
    return true;
#else
    OutError = TEXT("Windows shared memory transport is only available on Windows");
    return false;
#endif
}

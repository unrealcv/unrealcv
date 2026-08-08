#pragma once

#include "CoreMinimal.h"

struct FUnrealCVSharedMemoryView
{
    FString Name;
    int64 NumBytes = 0;
    uint64 Version = 0;
    int32 OffsetBytes = 0;
    void* Data = nullptr;
    double MapMs = 0.0;
    double MemcpyMs = 0.0;
    double TotalMs = 0.0;
};

class FUnrealCVSharedMemoryManager
{
  public:
    static FUnrealCVSharedMemoryManager& Get();

    bool WriteBytes(const FString& LogicalName, const void* SourceData, int64 NumBytes,
                    FUnrealCVSharedMemoryView& OutView, FString& OutError);
    bool Release(const FString& LogicalName);

  private:
    struct FSharedMemoryRegion;

    FString MakePlatformName(const FString& LogicalName, uint64 Version) const;
    bool CreateOrResizeRegion(const FString& LogicalName, int64 NumBytes, FUnrealCVSharedMemoryView& OutView,
                              FString& OutError);

    FCriticalSection CriticalSection;
    TMap<FString, TUniquePtr<FSharedMemoryRegion>> Regions;
    TMap<FString, uint64> RegionVersions;
};

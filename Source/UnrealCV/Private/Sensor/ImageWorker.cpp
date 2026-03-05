#include "ImageWorker.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "Runtime/Core/Public/HAL/PlatformProcess.h"
#include "UnrealcvLog.h"
#include "Utils/RuntimeConstants.h"

FImageWorker::FImageWorker() : Thread(nullptr)
{
	// Start();
}

void FImageWorker::Start()
{
	if (Thread == nullptr)
	{
		Thread = FRunnableThread::Create(this, TEXT("ImageWorker"), 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
	}
}

void FImageWorker::SaveFile(const TArray<FColor>& ImageData, const int Width, const int Height, const FString& Filename)
{
	// Should be called in the rendering thread or game thread
	check(IsInRenderingThread() || IsInGameThread());

	// Check the queue, if the
	FFrameData FrameData = {ImageData, Width, Height, Filename, GFrameNumber};
	UE_LOG(LogUnrealCV, Log, TEXT("Request to save frame number %d"), FrameData.FrameNumber);
	PendingData.Enqueue(FrameData);
}

uint32 FImageWorker::Run()
{
	while (!Stopping)
	{
		bool bProcessedFrame = false;
		FFrameData Frame;
		while (PendingData.Dequeue(Frame))
		{
			bProcessedFrame = true;
			UE_LOG(LogUnrealCV, Log, TEXT("Saving frame number %d"), Frame.FrameNumber);
			// ImageUtil.SavePngFile(Frame.ImageData, Frame.Width, Frame.Height, Frame.Filename);
			ImageUtil.SaveBmpFile(Frame.ImageData, Frame.Width, Frame.Height, Frame.Filename);
		}
		if (!bProcessedFrame)
		{
			FPlatformProcess::Sleep(UnrealcvRuntimeConstants::BusyWaitSleepSeconds);
		}
	}
	return 0;
}

void FImageWorker::Stop()
{
	Stopping = true;
}

/** Virtual destructor. */
FImageWorker::~FImageWorker()
{
	if (Thread != nullptr)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

#include "ImageWorker.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "UnrealcvLog.h"

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
		FFrameData Frame;
		while (PendingData.Dequeue(Frame))
		{
			UE_LOG(LogUnrealCV, Log, TEXT("Saving frame number %d"), Frame.FrameNumber);
			// ImageUtil.SavePngFile(Frame.ImageData, Frame.Width, Frame.Height, Frame.Filename);
			ImageUtil.SaveBmpFile(Frame.ImageData, Frame.Width, Frame.Height, Frame.Filename);
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

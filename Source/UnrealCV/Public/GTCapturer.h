#pragma once
#include "GTCapturer.generated.h"

/**
 * Use USceneCaptureComponent2D to export information from the scene.
 * This class needs to be tickable to update the rotation of the USceneCaptureComponent2D 
 */
UCLASS()
class UNREALCV_API UGTCapturer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
private:
	UGTCapturer();
	APawn* Pawn;

public:
	// UGTCapturer(APawn* Pawn);

	/** If Mode = "", Save normal image */
	static UGTCapturer* Create(APawn* Pawn, FString Mode=TEXT(""));

	static UMaterial* GetMaterial(FString ModeName);

	virtual void Tick(float DeltaTime) override; // TODO

	virtual bool IsTickable() const // whether this object will be ticked?
	{
		return bIsTicking;
	}
	virtual bool IsTickableWhenPaused() const
	{
		return bIsTicking;
	}
	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UGTCapturer, STATGROUP_Tickables);
	}
	bool IsPending()
	{
		return bIsPending;
	}

	bool Capture(FString Filename);

private:
	const bool bIsTicking = true;
	bool bIsPending = false;

	USceneCaptureComponent2D* CaptureComponent;
	FString Filename;
};
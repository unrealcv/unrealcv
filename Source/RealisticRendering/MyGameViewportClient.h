// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameViewportClient.h"
#include "MyGameViewportClient.generated.h"

/**
 *
 */
UCLASS()
class REALISTICRENDERING_API UMyGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	void CaptureScreen(const FString& Filename);
	UMyGameViewportClient();

protected:
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;

private:
	FEvent* EventWaitCapture;
	bool IsPendingSaveRequest = false;
	void DoCaptureScreen();
	FString CaptureFilename;
};

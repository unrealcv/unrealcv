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
	// FScopedEvent CaptureFinished; // TODO: Not sure FScopedEvent is the right thing to use
	UMyGameViewportClient();
	
protected:
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;
	
private:
	bool IsPendingSaveRequest = false;
	FString CaptureFilename;
};

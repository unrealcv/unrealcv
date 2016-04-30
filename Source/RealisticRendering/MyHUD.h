// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

/**
 * 
 */
UCLASS()
class REALISTICRENDERING_API AMyHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	AMyHUD();

	virtual void DrawHUD() override;

	// Toggle whether to draw the cursor
	static void ToggleCursor();
	static void ToggleLabel();

private:
	UPROPERTY()
	class UTexture2D* CrosshairTex;
	
	static bool bDrawCursor;
	static bool bDrawLabel;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/HUD.h"
#include "BoneVisualizationHUD.generated.h"

/**
 * 
 */
UCLASS()
class UNREALCV_API ABoneVisualizationHUD : public AHUD
{
	GENERATED_BODY()


		virtual void PostRender() override;
	
};

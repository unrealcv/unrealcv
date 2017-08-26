#pragma once

#include "CvCameraManager.generated.h"

class UCvCameraComponent;

UCLASS(Blueprintable)
class ACvCameraManager : public AActor
{
    GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	TArray<UCvCameraComponent*> GetCameras();
};

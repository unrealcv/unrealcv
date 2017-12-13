// Weichao Qiu @ 2017
#pragma once

#include "UE4CVWorldController.generated.h"

UCLASS()
class AUE4CVWorldController : public AActor
{
	GENERATED_BODY()

public:
	AUE4CVWorldController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
};

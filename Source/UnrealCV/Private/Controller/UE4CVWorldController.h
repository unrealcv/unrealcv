// Weichao Qiu @ 2017
#pragma once

#include "ObjectAnnotator.h"
#include "UE4CVWorldController.generated.h"

UCLASS()
class AUE4CVWorldController : public AActor
{
	GENERATED_BODY()

public:
	FObjectAnnotator ObjectAnnotator;

	AUE4CVWorldController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;


	/** Open new level */
	void OpenLevel(FName LevelName);
};

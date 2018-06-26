// Weichao Qiu @ 2017
#pragma once

#include "CoreMinimal.h"
#include "ObjectAnnotator.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Controller/PlayerViewMode.h"
#include "UE4CVWorldController.generated.h"

UCLASS()
class AUE4CVWorldController : public AActor
{
	GENERATED_BODY()

public:
	FObjectAnnotator ObjectAnnotator;

	UPROPERTY()
	UPlayerViewMode* PlayerViewMode;

	AUE4CVWorldController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/** Open new level */
	void OpenLevel(FName LevelName);
};

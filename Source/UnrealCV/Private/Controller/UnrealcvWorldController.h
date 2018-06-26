// Weichao Qiu @ 2017
#pragma once

#include "CoreMinimal.h"
#include "ObjectAnnotator.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Controller/PlayerViewMode.h"
#include "UnrealcvWorldController.generated.h"

UCLASS()
class AUnrealcvWorldController : public AActor
{
	GENERATED_BODY()

public:
	FObjectAnnotator ObjectAnnotator;

	UPROPERTY()
	UPlayerViewMode* PlayerViewMode;

	AUnrealcvWorldController(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	/** Open new level */
	void OpenLevel(FName LevelName);
};

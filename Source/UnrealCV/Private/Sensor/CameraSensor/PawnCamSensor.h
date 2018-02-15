// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "PawnCamSensor.generated.h"

/** Sensor to be attached to the player's view */
UCLASS(meta = (BlueprintSpawnableComponent))
class UPawnCamSensor : public UFusionCamSensor
{
	GENERATED_BODY()

public:
	UPawnCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);

	// Need to update the pawn as well
	/*
	virtual void SetWorldLocation(
		FVector NewLocation,
		bool bSweep,
		FHitResult * OutSweepHitResult,
		ETeleportType Teleport) override;
	
	virtual void SetWorldRotation(
		FRotator NewRotation,
		bool bSweep,
		FHitResult * OutSweepHitResult,
		ETeleportType Teleport) override;
		*/
};

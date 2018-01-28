// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "StereoCameraActor.generated.h"

UCLASS()
class AStereoCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AStereoCameraActor(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	UPROPERTY()
	float BaseLineDistance;

	UPROPERTY(Instanced, Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UFusionCamSensor* LeftSensor;

	UPROPERTY(Instanced, Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UFusionCamSensor* RightSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;
};

// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "CamSensorActor.h"
#include "FusionCameraActor.h"
#include "StereoCameraActor.generated.h"

UCLASS()
class UNREALCV_API AStereoCameraActor : public ACamSensorActor
{
	GENERATED_BODY()

public:
	// AStereoCameraActor(const FObjectInitializer& ObjectInitializer);
	AStereoCameraActor();

	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "unrealcv")
	float BaselineDistance;

	virtual TArray<FString> GetSensorNames() override;

	virtual TArray<UFusionCamSensor*> GetSensors() override;

private:
	UPROPERTY(Instanced, Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UFusionCamSensor* LeftSensor;

	UPROPERTY(Instanced, Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UFusionCamSensor* RightSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

	void SetBaselineDistance();
	
	void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent);
};

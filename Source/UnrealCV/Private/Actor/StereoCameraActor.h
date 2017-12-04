// Weichao Qiu @ 2017
#pragma once

#include "LitCamSensor.h"
#include "ObjMaskCamSensor.h"
#include "StereoCameraActor.generated.h"

UCLASS()
class AStereoCameraActor : public AActor
{
    GENERATED_BODY()

public:
	AStereoCameraActor(const FObjectInitializer& ObjectInitializer);

    // virtual void OnRegister() override;
	virtual void BeginPlay() override;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    ULitSensor* LeftLitSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    ULitSensor* RightLitSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UObjMaskSensor* LeftObjMaskSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UObjMaskSensor* RightObjMaskSensor;

	UPROPERTY(Category = StereoCameraActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SceneComponent;

    virtual void Tick(float DeltaSeconds) override;

};

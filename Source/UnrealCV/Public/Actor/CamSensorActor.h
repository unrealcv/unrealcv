// Weichao Qiu @ 2018
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CamSensorActor.generated.h"

UCLASS(abstract)
class UNREALCV_API ACamSensorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACamSensorActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual TArray<FString> GetSensorNames() { return {  }; }

	virtual TArray<class UFusionCamSensor*> GetSensors() { return { }; }

	int GetSensorNum() { return GetSensors().Num(); }
};

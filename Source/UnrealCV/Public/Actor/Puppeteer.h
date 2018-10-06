// Weichao Qiu @ 2018
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JsonObjectBP.h"
#include "Puppeteer.generated.h"

/**
A type of actor responsible for running and stopping the simulation in the scene.
*/
UCLASS(Abstract)
class UNREALCV_API APuppeteer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APuppeteer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// virtual void ResetSimulation() = 0;
	virtual void ResetSimulation() {}

	/** DataCaptureActor provides context to format data */
	virtual FJsonObjectBP GetState(class ADataCaptureActor* DataCaptureActor = nullptr) {
		return FJsonObjectBP();
	}
};

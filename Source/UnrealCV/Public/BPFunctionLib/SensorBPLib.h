// Weichao Qiu @ 2018
#pragma once

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "SensorBPLib.generated.h"

/** Sensor related function library */
UCLASS()
class UNREALCV_API USensorBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static TArray<UFusionCamSensor*> GetFusionSensorList();

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static UFusionCamSensor* GetSensorById(int SensorId);
};
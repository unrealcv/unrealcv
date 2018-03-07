// Weichao Qiu @ 2018
#include "UnrealCVPrivate.h"
#include "SensorBP.h"
#include "UE4CVServer.h"

TArray<UFusionCamSensor*> USensorBP::GetFusionSensorList()
{
	TArray<UFusionCamSensor*> SensorList;

	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	if (!World) return SensorList;

	TArray<UActorComponent*> PawnComponents =  FUE4CVServer::Get().GetPawn()->GetComponentsByClass(UFusionCamSensor::StaticClass());
	// Make sure the one attached to the pawn is the first one.
	for (UActorComponent* FusionCamSensor : PawnComponents)
	{
		SensorList.Add(Cast<UFusionCamSensor>(FusionCamSensor));
	}

	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = false;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(UFusionCamSensor::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	// Filter out objects not belong to the game world (editor world for example)
	for (UObject* SensorObject : UObjectList)
	{
		UFusionCamSensor *FusionSensor = Cast<UFusionCamSensor>(SensorObject);
		if (FusionSensor->GetWorld() != World) continue;
		if (SensorList.Contains(FusionSensor) == false)
		{
			SensorList.Add(FusionSensor);
		}
	}
	return SensorList;
}

UFusionCamSensor* USensorBP::GetSensorById(int SensorId)
{
	TArray<UFusionCamSensor*> SensorList = USensorBP::GetFusionSensorList();
	if (SensorId < 0 || SensorId >= SensorList.Num()) return nullptr;
	return SensorList[SensorId];
}

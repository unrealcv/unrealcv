#include "UnrealCVPrivate.h"
#include "UnrealCV.h"
#include "CvCameraManager.h"
#include "CvCameraComponent.h"
#include "EngineUtils.h"

ACvCameraManager::ACvCameraManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

TArray<UCvCameraComponent*> ACvCameraManager::GetCameras()
{
	ScreenLog("Get a list of all cameras");
	TArray<UCvCameraComponent*> Cameras;

	for (TObjectIterator<UCvCameraComponent> Itr; Itr; ++Itr)
	{
		UCvCameraComponent *Camera = *Itr;
		
		if (!Camera->IsTemplate() && Camera->GetWorld()->WorldType != EWorldType::Editor)
		{
			Cameras.Add(Camera);
		}
	}
	ScreenLog(FString::Printf(TEXT("%d cameras in the scene"), Cameras.Num()));
	return Cameras;
}

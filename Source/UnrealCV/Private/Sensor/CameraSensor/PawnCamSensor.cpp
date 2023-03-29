// Weichao Qiu @ 2017

#include "PawnCamSensor.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
// #include "LitCamSensor.h"
#include "BaseCameraSensor.h"

UPawnCamSensor::UPawnCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (!IsValid(Sensor)) continue;
		// Sensor->bAbsoluteLocation = true;
		// Sensor->bAbsoluteRotation = true;
	}
}

void UPawnCamSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T)
{
	Super::TickComponent(DeltaTime, TickType, T);
	// Make the world location and view rotation is the same as the Pawn viewpoint
	FVector EyeLocation;
	FRotator EyeRotation;

	AActor *Owner = this->GetOwner();
	APawn *Pawn = Cast<APawn>(Owner);
	if (!IsValid(Pawn))
	{
		return;
	}

	// I want to get the Viewport, not really the pawn view.
	// Note: https://answers.unrealengine.com/questions/5155/getting-editor-viewport-camera.html
	// GEngine->GameViewport->Viewport
	Pawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	// GetPlayerViewpoint(EyeLocation, EyeRotation);
	// FRotator CompRotation = this->GetComponentRotation();
	// FVector CompLocation = this->GetComponentLocation();
	// ScreenLog(FString::Printf(TEXT("Rotation, Actor Eye: %s, Component: %s"), *EyeRotation.ToString(), *CompRotation.ToString()));
	// ScreenLog(FString::Printf(TEXT("Location, Actor Eye: %s, Component: %s"), *EyeLocation.ToString(), *CompLocation.ToString()));

	// TODO: any alternative implementation
	this->SetWorldLocation(EyeLocation);
	this->SetWorldRotation(EyeRotation);
	this->UpdateChildTransforms();

	// USceneComponent* Parent = this->LitCamSensor->GetAttachParent();
	// if (Parent != this)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("Unexpected attach"));
	// }
	// FTransform RelativeTransform = this->LitCamSensor->GetRelativeTransform();
	// FVector LitLocation = this->LitCamSensor->GetSensorLocation();
	// FRotator LitRotation = this->LitCamSensor->GetSensorRotation();
	// FVector ThisLocation = this->GetSensorLocation();
	// FRotator ThisRotation = this->GetSensorRotation();
	// this->LitCamSensor->SetSensorLocation(EyeLocation);
	// this->LitCamSensor->SetSensorRotation(EyeRotation);

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (!IsValid(Sensor)) continue;
		Sensor->SetSensorLocation(EyeLocation);
		Sensor->SetSensorRotation(EyeRotation);
	}
	// TODO: check this with a player pawn
	// if (CompLocation != EyeLocation || CompRotation != EyeRotation)
	// {
	// ScreenLog(TEXT("Pawn Camera Sensor is not correctly mounted, location or rotation mismatch"));
	// }

}

void UPawnCamSensor::SetSensorLocation(FVector Location)
{
	// Super::SetSensorLocation(NewLocation, bSweep, OutSweepHitResult, Teleport);
	AActor *Owner = this->GetOwner();
	APawn *Pawn = Cast<APawn>(Owner);
	if (!IsValid(Pawn))
	{
		return;
	}
	bool Sweep = false;
	Pawn->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);
}

void UPawnCamSensor::SetSensorRotation(FRotator Rotation)
{
	AActor *Owner = this->GetOwner();
	APawn *Pawn = Cast<APawn>(Owner);
	if (!IsValid(Pawn))
	{
		return;
	}
	AController* Controller = Pawn->GetController();
	Controller->ClientSetRotation(Rotation);
}

// Weichao Qiu @ 2017

#include "UnrealCVPrivate.h"
#include "PawnCamSensor.h"

UPawnCamSensor::UPawnCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;
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

	Pawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FRotator CompRotation = this->GetComponentRotation();
	FVector CompLocation = this->GetComponentLocation();
	// ScreenLog(FString::Printf(TEXT("Rotation, Actor Eye: %s, Component: %s"), *EyeRotation.ToString(), *CompRotation.ToString()));
	// ScreenLog(FString::Printf(TEXT("Location, Actor Eye: %s, Component: %s"), *EyeLocation.ToString(), *CompLocation.ToString()));

	// TODO: any alternative implementation
	this->SetWorldRotation(EyeRotation);
	CompRotation = this->GetComponentRotation(); // Get updated value

	// TODO: check this with a player pawn
	if (CompLocation != EyeLocation || CompRotation != EyeRotation)
	{
		// ScreenLog(TEXT("Pawn Camera Sensor is not correctly mounted, location or rotation mismatch"));
	}

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
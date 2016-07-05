// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "UE4CVCharacter.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"
#include "UnrealCV.h"

AUE4CVCharacter::AUE4CVCharacter()
{
	// Pending tasks in UE4CVServer will be processed in Tick(), so bCanEverTick needs to be enabled
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUE4CVCharacter::BeginPlay()
{
	Super::BeginPlay();
	FUE4CVServer::Get().Init(this);
}

// Called every frame
void AUE4CVCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	FUE4CVServer::Get().ProcessPendingRequest();
	// Send camera info for every movement, can be potentially useful for recording movie
	/*
	FExecStatus ExecStatus = FUE4CVServer::Get().CommandDispatcher->Exec("vget /camera/0/location");
	ExecStatus += FUE4CVServer::Get().CommandDispatcher->Exec("vget /camera/0/rotation");
	FUE4CVServer::Get().SendClientMessage(ExecStatus.GetMessage());
	*/
}

// Called to bind functionality to input
// Input configuration is following the default of FirstPersonShooter game
void AUE4CVCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AUE4CVCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AUE4CVCharacter::MoveRight);

	// Handle Mouse Input
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AUE4CVCharacter::OnFire);
}

void AUE4CVCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AUE4CVCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AUE4CVCharacter::OnFire()
{
	// Send a message to notify client an event just happened
	FUE4CVServer::Get().SendClientMessage("clicked");
	UE_LOG(LogTemp, Warning, TEXT("Mouse Clicked"));
}

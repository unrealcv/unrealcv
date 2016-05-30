// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "UE4CVCharacter.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"
#include "UnrealCV.h"

// Sets default values
AUE4CVCharacter::AUE4CVCharacter()
{

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AUE4CVCharacter::BeginPlay()
{
	Super::BeginPlay();

	Server = new FUE4CVServer(this);
	Server->Start();
	/*
	FViewMode::Get().SetWorld(this->GetWorld());
	FObjectPainter::Get().SetLevel(this->GetLevel());
	FObjectPainter::Get().PaintRandomColors();

	Commands = new UE4CVCommands(this, &CommandDispatcher);
	Server = new FUE4CVServer(&CommandDispatcher);
	Server->Start();
	NetworkManager = Server->NetworkManager; // I need this to show information on screen.

	ConsoleOutputDevice = new FConsoleOutputDevice(GetWorld()->GetGameViewport()->ViewportConsole); 
	// TODO: Check the pointers
	// Register commands to UE console
	ConsoleHelper = new FConsoleHelper(&CommandDispatcher, ConsoleOutputDevice);
	*/
}

void AUE4CVCharacter::TakeScreenShot()
{
	FExecStatus ExecStatus = FExecStatus::OK();
	FCallbackDelegate CallbackDelegate;
	CallbackDelegate.BindLambda([this](FExecStatus ExecStatus)
	{
		FExecStatus ExecStatusCombine = ExecStatus;
		ExecStatusCombine += CommandDispatcher.Exec("vget /camera/0/location");
		ExecStatusCombine += CommandDispatcher.Exec("vget /camera/0/rotation");
		Server->SendClientMessage(ExecStatusCombine.GetMessage());
	});
	CommandDispatcher.ExecAsync("vget /camera/0/view", CallbackDelegate);
}

// Called every frame
void AUE4CVCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	Server->ProcessPendingRequest();
}

// Called to bind functionality to input
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
	TakeScreenShot();
	UE_LOG(LogTemp, Warning, TEXT("Fire"));
}


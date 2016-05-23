// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "MyCharacter.h"
// #include "MyHUD.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"
#include "ViewMode.h"
// #include "MyGameViewportClient.h"
#include "Tester.h"
#include "UE4CVCommands.h"
// #include "IUObjectPlugin.h"
// #include "UnrealCV.h"
// #include "Console.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	Commands = new UE4CVCommands(this, &CommandDispatcher);

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	// Tester = new NetworkManagerTester();
	// Tester = new UE4CVServerTester(&CommandDispatcher);
	Tester = new FilePathTester();
	// Tester->Init();
	Super::BeginPlay();

	FViewMode::Get().SetWorld(this->GetWorld());
	Server = new FUE4CVServer(&CommandDispatcher);
	Server->Start();
	NetworkManager = Server->NetworkManager; // I need this to show information on screen.

	ConsoleOutputDevice = new FConsoleOutputDevice(GetWorld()->GetGameViewport()->ViewportConsole); // TODO: Check the pointers
	// Register commands to UE console
	ConsoleHelper = new FConsoleHelper(&CommandDispatcher, ConsoleOutputDevice);

	// RegisterCommands();

	// PaintRandomColors(TArray<FString>());
}

void AMyCharacter::NotifyClient(FString Message)
{
	// Send a message to client to say a new frame is rendered.
	Server->SendClientMessage(Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

void AMyCharacter::TakeScreenShot()
{
	FExecStatus ExecStatus = FExecStatus::OK();
	FCallbackDelegate CallbackDelegate;
	CallbackDelegate.BindLambda([this](FExecStatus ExecStatus)
	{
		FExecStatus ExecStatusCombine = ExecStatus;
		ExecStatusCombine += CommandDispatcher.Exec("vget /camera/0/location");
		ExecStatusCombine += CommandDispatcher.Exec("vget /camera/0/rotation");
		NotifyClient(ExecStatusCombine.GetMessage());
	});
	CommandDispatcher.ExecAsync("vget /camera/0/view", CallbackDelegate);
	// CommandDispatcher.ExecAsync("vget /camera/0/location", CallbackDelegate);
	// TODO: Implement operator + for FExecStatus
}

// Called every frame
void AMyCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	Server->ProcessPendingRequest();
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	// Handle Mouse Input
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AMyCharacter::OnFire);
}

FExecStatus AMyCharacter::BindAxisWrapper(const TArray<FString>& Args)
{
	// FInputAxisHandlerSignature::TUObjectMethodDelegate< UserClass >::FMethodPtr
	return FExecStatus::OK();
}

void AMyCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyCharacter::OnFire()
{
	if (Tester)
	{
		Tester->Run();
	}
	// PaintAllObjects(TArray<FString>());
	TakeScreenShot();

	// ParseMaterialConfiguration();
	// TestMaterialLoading();

	UE_LOG(LogTemp, Warning, TEXT("Fire"));
	FHitResult HitResult;
	// The original version for the shooting game use CameraComponent
	FVector StartLocation = GetActorLocation();
	FRotator Direction = GetActorRotation();

	FVector EndLocation = StartLocation + Direction.Vector() * 10000;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();

		// UE_LOG(LogTemp, Warning, TEXT("%s"), *HitActor->GetActorLabel());
		// Draw a bounding box of the hitted object and also output the name of it.
	}
}


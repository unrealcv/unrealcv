// Weichao Qiu @ 2016
#include "UnrealcvGameMode.h"
#include <string>
#include "ImageUtils.h"
#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

AUnrealcvGameMode::AUnrealcvGameMode()
{
	DefaultPawnClass = AUnrealcvPawn::StaticClass();
}

// TODO: Remove the requirement of modifying pawn
/**
  * UnrealcvCharacter acts like a walking human
  */
AUnrealcvCharacter::AUnrealcvCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUnrealcvCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AUnrealcvCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AUnrealcvCharacter::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AUnrealcvCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AUnrealcvCharacter::MoveRight);

	// Handle Mouse Input
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AUnrealcvCharacter::OnFire);
}

void AUnrealcvCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AUnrealcvCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AUnrealcvCharacter::OnFire()
{
	// Send a message to notify client an event just happened
	UE_LOG(LogUnrealCV, Warning, TEXT("Mouse Clicked"));
}


/**
 * UnrealcvPawn can move freely in the 3D space
 */
AUnrealcvPawn::AUnrealcvPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->SetHidden(true);
	this->SetTickableWhenPaused(true);
	// this->CollisionComponent->SetSphereRadius(1.0f);
}

void AUnrealcvPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AUnrealcvPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	// FUnrealcvServer::Get().ProcessPendingRequest();
	FRotator ControlRotation = this->Controller->GetControlRotation();
	this->SetActorRotation(ControlRotation);
}

void AUnrealcvPawn::SetupPlayerInputComponent(class UInputComponent* InInputComponent)
{
	Super::SetupPlayerInputComponent(InInputComponent);

	InputComponent->BindKey(FKey("LeftMouseButton"), IE_Pressed, this, &AUnrealcvPawn::OnFire);
}

void AUnrealcvPawn::OnFire()
{
	// Send a message to notify client an event just happened
	UE_LOG(LogUnrealCV, Warning, TEXT("Mouse Clicked"));
}

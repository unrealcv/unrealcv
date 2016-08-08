#include "UnrealCVPrivate.h"
#include "UE4CVGameMode.h"
#include "UnrealCV.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"

AUE4CVGameMode::AUE4CVGameMode()
{
	DefaultPawnClass = AUE4CVPawn::StaticClass();
}

/**
  * UE4CVCharacter acts like a walking human
  */
AUE4CVCharacter::AUE4CVCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AUE4CVCharacter::BeginPlay()
{
	Super::BeginPlay();
	FUE4CVServer::Get().Init(this);
}

void AUE4CVCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	FUE4CVServer::Get().ProcessPendingRequest();
}

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


/**
 * UE4CVPawn can move freely in the 3D space
 */
AUE4CVPawn::AUE4CVPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	this->CollisionComponent->SetSphereRadius(1.0f);
}

void AUE4CVPawn::BeginPlay()
{
	Super::BeginPlay();
	FUE4CVServer::Get().Init(this);
}

void AUE4CVPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	FUE4CVServer::Get().ProcessPendingRequest();
}

void AUE4CVPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindKey(FKey("LeftMouseButton"), IE_Pressed, this, &AUE4CVPawn::OnFire);
}

void AUE4CVPawn::OnFire()
{
	// Send a message to notify client an event just happened
	FUE4CVServer::Get().SendClientMessage("clicked");
	UE_LOG(LogTemp, Warning, TEXT("Mouse Clicked"));
}

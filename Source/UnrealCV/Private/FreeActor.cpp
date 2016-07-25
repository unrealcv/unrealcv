#include "UnrealCVPrivate.h"
#include "FreeActor.h"

AFreeActor::AFreeActor()
{
    PrimaryActorTick.bCanEverTick = true;

	// A root component is required to enable interaction
	// See https://docs.unrealengine.com/latest/INT/Programming/Tutorials/Components/1/index.html for more detail.

	USphereComponent* SphereComponent = this->CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	RootComponent = SphereComponent;
	SphereComponent->InitSphereRadius(1.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));
	// A list of collision profile is in https://docs.unrealengine.com/latest/INT/Engine/Physics/Collision/Reference/index.html
}

void AFreeActor::BeginPlay()
{
	Super::BeginPlay();
	FUE4CVServer::Get().Init(this);
}

void AFreeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FUE4CVServer::Get().ProcessPendingRequest();
}

void AFreeActor::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindKey(FKey("w"), EInputEvent::IE_Pressed, this, &AFreeActor::Forward);
	InputComponent->BindKey(FKey("s"), EInputEvent::IE_Pressed, this, &AFreeActor::Backward);
	InputComponent->BindKey(FKey("a"), EInputEvent::IE_Pressed, this, &AFreeActor::Left);
	InputComponent->BindKey(FKey("d"), EInputEvent::IE_Pressed, this, &AFreeActor::Right);
	InputComponent->BindKey(FKey("q"), EInputEvent::IE_Pressed, this, &AFreeActor::Up);
	InputComponent->BindKey(FKey("e"), EInputEvent::IE_Pressed, this, &AFreeActor::Down);

	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AFreeActor::OnFire);
}

void AFreeActor::Forward()
{
	FVector Location = GetActorLocation();
	Location.Y -= 10;
	MoveTo(Location);
}

void AFreeActor::Backward()
{
	FVector Location = GetActorLocation();
	Location.Y += 10;
	MoveTo(Location);
}

void AFreeActor::Left()
{
	FVector Location = GetActorLocation();
	Location.X -= 10;
	MoveTo(Location);
}

void AFreeActor::Right()
{
	FVector Location = GetActorLocation();
	Location.X += 10;
	MoveTo(Location);
}

void AFreeActor::Up()
{
	FVector Location = GetActorLocation();
	Location.Z += 10;
	MoveTo(Location);
}

void AFreeActor::Down()
{
	FVector Location = GetActorLocation();
	Location.Z -= 10;
	MoveTo(Location);
}

void AFreeActor::MoveTo(FVector NewLocation)
{
	bool Sweep = true;
	// if sweep is true, the object can not move through another object
	bool Success = SetActorLocation(NewLocation, Sweep, NULL, ETeleportType::TeleportPhysics);
}

void AFreeActor::OnFire()
{
	// Send a message to notify client an event just happened
	FUE4CVServer::Get().SendClientMessage("clicked");
	UE_LOG(LogTemp, Warning, TEXT("Mouse Clicked"));
}

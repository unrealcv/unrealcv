#include "UnrealCVPrivate.h"
#include "UE4CVPawn.h"
#include "UnrealCV.h"

// Sets default values
AUE4CVPawn::AUE4CVPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AUE4CVPawn::BeginPlay()
{
	Super::BeginPlay();
	FUE4CVServer::Get().Init(this);
}

// Called every frame
void AUE4CVPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	FUE4CVServer::Get().ProcessPendingRequest();
}

// Called to bind functionality to input
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
	// TODO: Add this to console output.
}

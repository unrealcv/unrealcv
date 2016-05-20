// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "MyCharacter.h"
#include "MyHUD.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"
#include "ViewMode.h"
#include "MyGameViewportClient.h"
#include "Tester.h"
#include "UE4CVCommands.h"
// #include "Console.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	Commands = new UE4CVCommands(this);

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

	FViewMode::Get().World = this->GetWorld();
	Server = new FUE4CVServer(&CommandDispatcher);
	Server->Start();
	NetworkManager = Server->NetworkManager; // I need this to show information on screen.

	ConsoleOutputDevice = new FConsoleOutputDevice(GetWorld()->GetGameViewport()->ViewportConsole); // TODO: Check the pointers
	// Register commands to UE console
	ConsoleHelper = new FConsoleHelper(&CommandDispatcher, ConsoleOutputDevice);

	// RegisterCommands();

	PaintRandomColors(TArray<FString>());
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

FExecStatus AMyCharacter::PaintRandomColors(const TArray<FString>& Args)
{
	// Iterate over all actors
	ULevel* Level = GetLevel();

	// Get a random color
	for (auto Actor : Level->Actors)
	{
		if (Actor && Actor->IsA(AStaticMeshActor::StaticClass())) // Only StaticMeshActor is interesting
		{
			// FString ActorLabel = Actor->GetActorLabel();
			FString ActorLabel = Actor->GetHumanReadableName();
			FColor NewColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
			ObjectsColorMapping.Emplace(ActorLabel, NewColor);
			// if (Actor->GetActorLabel() == FString("SM_Door43"))
			{
				PaintObject(Actor, NewColor);
			}
		}
	}

	// Paint actor using floodfill.
	return FExecStatus::OK();
}

bool AMyCharacter::PaintObject(AActor* Actor, const FColor& NewColor)
{
	if (!Actor) return false;

	TArray<UMeshComponent*> PaintableComponents;
	// TInlineComponentArray<UMeshComponent*> MeshComponents;
	// Actor->GetComponents<UMeshComponent>(MeshComponents);
	Actor->GetComponents<UMeshComponent>(PaintableComponents);


	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			if (UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh)
			{
				uint32 PaintingMeshLODIndex = 0;
				uint32 NumLODLevel = StaticMeshComponent->StaticMesh->RenderData->LODResources.Num();
				check(NumLODLevel == 1);
				FStaticMeshLODResources& LODModel = StaticMeshComponent->StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
				FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

				// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied
				StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
				InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

				// Setup OverrideVertexColors
				if (!InstanceMeshLODInfo->OverrideVertexColors) {
					InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

					FColor FillColor = FColor(255, 255, 255, 255);
					InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::White, LODModel.GetNumVertices());
				}

				uint32 NumVertices = LODModel.GetNumVertices();
				check(InstanceMeshLODInfo->OverrideVertexColors);
				check(NumVertices <= InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices());
				// StaticMeshComponent->CachePaintedDataIfNecessary();

				for (uint32 ColorIndex = 0; ColorIndex < NumVertices; ++ColorIndex)
				{
					// FColor NewColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
					// LODModel.ColorVertexBuffer.VertexColor(ColorIndex) = NewColor;  // This is vertex level
					// Need to initialize the vertex buffer first
					uint32 NumOverrideVertexColors = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices();
					uint32 NumPaintedVertices = InstanceMeshLODInfo->PaintedVertices.Num();
					// check(NumOverrideVertexColors == NumPaintedVertices);
					InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = NewColor;
					// InstanceMeshLODInfo->PaintedVertices[ColorIndex].Color = NewColor;
				}
				BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
				StaticMeshComponent->MarkRenderStateDirty();
				// BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);


				/*
				// TODO: Need to check other LOD levels
				// Use flood fill to paint mesh vertices
				UE_LOG(LogTemp, Warning, TEXT("%s:%s has %d vertices"), *Actor->GetActorLabel(), *StaticMeshComponent->GetName(), NumVertices);

				if (LODModel.ColorVertexBuffer.GetNumVertices() == 0)
				{
					// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
					LODModel.ColorVertexBuffer.InitFromSingleColor(FColor(255, 255, 255, 255), LODModel.GetNumVertices());
				}

				*/
			}
		}
	}
	return true;
}

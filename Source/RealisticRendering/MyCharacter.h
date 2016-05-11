// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "CommandDispatcher.h"
#include "UE4CVServer.h"
#include "ConsoleHelper.h"
#include "MyCharacter.generated.h"

UCLASS()
// TODO: Make this programmable with blueprint
class REALISTICRENDERING_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMyCharacter();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	// Move forward
	void MoveForward(float Value);

	// Rotate
	void MoveRight(float Value);
	
	// Define Console Commands
	void DefineConsoleCommands();

	// Fire
	void OnFire();



	void NotifyClient(FString Message);

	void TakeScreenShot();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UNetworkManager* NetworkManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 Health;
private:
	// UPROPERTY() // This is super important to prevent the NetworkManager from GC.
	// UNetworkManager* NetworkManager;
	FCommandDispatcher CommandDispatcher;
	FUE4CVServer* Server;
	FConsoleHelper* ConsoleHelper;
	FConsoleOutputDevice* ConsoleOutputDevice;
	TMap<FString, FColor> ObjectsColorMapping;
	TMap<FString, AActor*> ObjectsMapping;

	void RegisterCommands();

	FExecStatus BindAxisWrapper(const TArray<FString>& Args);
	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	FExecStatus GetCameraImage(const TArray<FString>& Args);

	FExecStatus GetObjects(const TArray<FString>& Args);
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	FExecStatus SetObjectColor(const TArray<FString>& Args);
	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);
	FExecStatus GetCommands(const TArray<FString>& Args);

	// Vertex One Object with Flood-Fill
	bool PaintObject(AActor* Actor, const FColor& NewColor);
	// Paint all objects
	FExecStatus PaintRandomColors(const TArray<FString>& Args);

	// Test functions for learning and development
	void TestMaterialLoading();
	void TestDispatchCommands();
	void ParseMaterialConfiguration();
};


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "CommandDispatcher.h"
#include "UE4CVServer.h"
#include "ConsoleHelper.h"
#include "MyCharacter.generated.h"

UCLASS()
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

	// Paint all objects
	FExecStatus PaintAllObjects(const TArray<FString>& Args);

	// Vertex One Object with Flood-Fill
	void PaintObject(AActor* Actor);

	void NotifyClient(FString Message);

	void TakeScreenShot();

	void DispatchCommands();
private:
	// UPROPERTY() // This is super important to prevent the NetworkManager from GC.
	// UNetworkManager* NetworkManager;
	FCommandDispatcher CommandDispatcher;
	FUE4CVServer* Server;
	FConsoleHelper* ConsoleHelper;
	FConsoleOutputDevice* ConsoleOutputDevice;

	FExecStatus BindAxisWrapper(const TArray<FString>& Args);
	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	FExecStatus GetCameraImage(const TArray<FString>& Args);


	// Test functions for learning and development
	void TestMaterialLoading();
	void TestDispatchCommands();
	void ParseMaterialConfiguration();
};


// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "CommandDispatcher.h"
#include "UE4CVServer.h"
#include "ConsoleHelper.h"
#include "MyCharacter.generated.h"

class UE4CVCommands;
class ITester;
UCLASS()
// TODO: Make this programmable with blueprint
class REALISTICRENDERING_API AMyCharacter : public ACharacter
{
	friend class UE4CVCommands;
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
	UE4CVCommands* Commands;
	ITester* Tester;
	// UPROPERTY() // This is super important to prevent the NetworkManager from GC.
	// UNetworkManager* NetworkManager;
	FCommandDispatcher CommandDispatcher;
	FUE4CVServer* Server;
	FConsoleHelper* ConsoleHelper;
	FConsoleOutputDevice* ConsoleOutputDevice;
	TMap<FString, FColor> ObjectsColorMapping;
	TMap<FString, AActor*> ObjectsMapping;

	FExecStatus BindAxisWrapper(const TArray<FString>& Args);

	// Vertex One Object with Flood-Fill
	bool PaintObject(AActor* Actor, const FColor& NewColor);
	// Paint all objects
	FExecStatus PaintRandomColors(const TArray<FString>& Args);

};

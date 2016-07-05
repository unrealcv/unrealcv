// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Character.h"
#include "UnrealCV.h"
#include "UE4CVCharacter.generated.h"


UCLASS()
// TODO: Make this programmable with blueprint
class AUE4CVCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AUE4CVCharacter();

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
};

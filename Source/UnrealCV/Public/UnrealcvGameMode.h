// Weichao Qiu @ 2016
#pragma once

#include "Runtime/Engine/Classes/GameFramework/GameMode.h"
#include "Runtime/Engine/Classes/GameFramework/DefaultPawn.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "UnrealcvGameMode.generated.h"

/**
 *
 */
UCLASS()
class UNREALCV_API AUnrealcvGameMode : public AGameMode
{
	GENERATED_BODY()
	AUnrealcvGameMode();
};

/**
 * UnrealcvPawn can move freely in the 3D space
 */
UCLASS()
class UNREALCV_API AUnrealcvPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AUnrealcvPawn();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void OnFire();
};

/**
 * UnrealcvCharacter acts like a walking human
 */
UCLASS()
class UNREALCV_API AUnrealcvCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUnrealcvCharacter();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void OnFire();
	void NotifyClient(FString Message);
};

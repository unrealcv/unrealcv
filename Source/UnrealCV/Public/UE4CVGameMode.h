#pragma once

#include "UnrealCVPrivate.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/Pawn.h"
#include "UE4CVGameMode.generated.h"

/**
 *
 */
UCLASS()
class UNREALCV_API AUE4CVGameMode : public AGameMode
{
	GENERATED_BODY()
	AUE4CVGameMode();
};

/**
 * UE4CVPawn can move freely in the 3D space
 */
UCLASS()
class UNREALCV_API AUE4CVPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AUE4CVPawn();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void OnFire();
};

/**
 * UE4CVCharacter acts like a walking human
 */
UCLASS()
class UNREALCV_API AUE4CVCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AUE4CVCharacter();
	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void OnFire();
	void NotifyClient(FString Message);
};

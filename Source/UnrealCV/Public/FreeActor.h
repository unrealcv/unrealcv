#pragma once

#include "UnrealCV.h"
#include "FreeActor.generated.h"

UCLASS()
class AFreeActor : public APawn
{
    GENERATED_BODY()
public:
    AFreeActor();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaSeconds) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

    void Forward();
    void Backward();
    void Left();
    void Right();
    void Up();
    void Down();
    void MoveTo(FVector NewLocation);
    void OnFire();
};

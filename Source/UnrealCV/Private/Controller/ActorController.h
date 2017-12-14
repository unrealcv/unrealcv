// Weichao Qiu @ 2017
#pragma once

class FActorController
{
public:
    FActorController(AActor* InActor);

    FRotator GetRotation();

    FVector GetLocation();

    FColor GetAnnotaionColor();

    bool SetLocation(FVector NewLocation);

    bool SetRotation(FRotator NewRotation);

private:
    AActor* Actor;

};

// Weichao Qiu @ 2017

#pragma once

class FVertexSensor
{
public:
    FVertexSensor(AActor* InActor);

    TArray<FVector> GetVertexArray();

private:
    AActor* OwnerActor;
};

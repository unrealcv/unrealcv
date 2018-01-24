// Weichao Qiu @ 2017

#pragma once

class FVertexSensor
{
public:
    FVertexSensor(const AActor* InActor);

    TArray<FVector> GetVertexArray();

private:
    const AActor* OwnerActor;
};

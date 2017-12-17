// Weichao Qiu @ 2017
#pragma once

class FActorController
{
public:
	FActorController(AActor* InActor);

	FRotator GetRotation();
	bool SetRotation(FRotator NewRotation);

	FVector GetLocation();
	bool SetLocation(FVector NewLocation);

	// FColor GetAnnotaionColor();

private:
	AActor* Actor;

};

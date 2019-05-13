// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/GameFramework/Actor.h"

class FActorController
{
public:
	FActorController(AActor* InActor);

	FVector GetLocation();
	void SetLocation(FVector NewLocation);

	FRotator GetRotation();
	void SetRotation(FRotator NewRotation);

	EComponentMobility::Type GetMobility();

	void Show();
	void Hide();

	void GetAnnotationColor(FColor& AnnotationColor);
	void SetAnnotationColor(const FColor& AnnotationColor);

	// FColor GetAnnotaionColor();

private:
	AActor* Actor;

};

// Weichao Qiu @ 2017
#pragma once

class FActorController
{
public:
	FActorController(AActor* InActor);

	FVector GetLocation();
	bool SetLocation(FVector NewLocation);

	FRotator GetRotation();
	bool SetRotation(FRotator NewRotation);

	EComponentMobility::Type GetMobility();

	void Show();
	void Hide();

	bool GetAnnotationColor(FColor& AnnotationColor);
	bool SetAnnotationColor(const FColor& AnnotationColor);

	// FColor GetAnnotaionColor();

private:
	AActor* Actor;

};

#include "UnrealCVPrivate.h"
#include "UE4CVGameMode.h"
#include "UnrealCV.h"

AUE4CVGameMode::AUE4CVGameMode()
{
	// DefaultPawnClass = AUE4CVCharacter::StaticClass();
	// DefaultPawnClass = ADefaultPawn::StaticClass();
	DefaultPawnClass = AUE4CVPawn::StaticClass();
}

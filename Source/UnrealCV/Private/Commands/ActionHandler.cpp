#include "ActionHandler.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerInput.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"

#include "WorldController.h"
#include "VisionBPLib.h"

void FActionHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::PauseGame);
	Help = "Pause the game";
	CommandDispatcher->BindCommand("vset /action/game/pause", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::ResumeGame);
	Help = "Resume the game";
	CommandDispatcher->BindCommand("vset /action/game/resume", Cmd, Help);

	CommandDispatcher->BindCommand(
		"vget /action/game/is_paused",
		FDispatcherDelegate::CreateRaw(this, &FActionHandler::GetIsPaused),
		"Get the pause status"
	);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::OpenLevel);
	Help = "Open level";
	CommandDispatcher->BindCommand("vset /action/game/level [str]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::EnableInput);
	Help = "Enable input";
	CommandDispatcher->BindCommand("vset /action/input/enable", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::DisableInput);
	Help = "Disable input";
	CommandDispatcher->BindCommand("vset /action/input/disable", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::SetStereoDistance);
	Help = "Set the distance of binocular stereo camera";
	CommandDispatcher->BindCommand("vset /action/eyes_distance [float]", Cmd, Help);


	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionHandler::Keyboard);
	Help = "Send a keyboard action to the game";
	CommandDispatcher->BindCommand("vset /action/keyboard [str] [float]", Cmd, Help);

	CommandDispatcher->BindCommand(
		"vset /action/clean_garbage",
		FDispatcherDelegate::CreateRaw(this, &FActionHandler::GarbageCollection),
		"Manually collect garbage in the RAM"
	);

	CommandDispatcher->BindCommand(
		"vset /action/set_fixed_frame_rate [float]",
		FDispatcherDelegate::CreateRaw(this, &FActionHandler::SetFixedFPS),
		"Set fixed frame rate of the world"
	);
}

FExecStatus FActionHandler::PauseGame(const TArray<FString>& Args)
{
	APlayerController* PlayerController = this->GetWorld()->GetFirstPlayerController();
	// PlayerController->Pause();
	PlayerController->SetPause(true);
	return FExecStatus::OK();
}

FExecStatus FActionHandler::ResumeGame(const TArray<FString>& Args)
{
	APlayerController* PlayerController = this->GetWorld()->GetFirstPlayerController();
	// PlayerController->Pause();
	PlayerController->SetPause(false);
	return FExecStatus::OK();
}

FExecStatus FActionHandler::GetIsPaused(const TArray<FString>& Args)
{
	APlayerController* PlayerController = this->GetWorld()->GetFirstPlayerController();
	// PlayerController->Pause();
	bool bIsPaused = PlayerController->IsPaused();
	// PlayerController->SetPause(false);
	return FExecStatus::OK(bIsPaused ? TEXT("true") : TEXT("false"));
}


FExecStatus FActionHandler::OpenLevel(const TArray<FString>& Args)
{
	if (Args.Num() == 1) // Level name
	{
		FString LevelName = Args[0];
		FUnrealcvServer::Get().WorldController->OpenLevel(FName(*LevelName));
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error("Expect argument: level name");
	}
}

FExecStatus FActionHandler::EnableInput(const TArray<FString>& Args)
{
	APawn* Pawn = FUnrealcvServer::Get().GetPawn();
	if (!IsValid(Pawn))
	{
		UVisionBPLib::UpdateInput(Pawn, true);
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error("The pawn is invalid");
	}
}

FExecStatus FActionHandler::DisableInput(const TArray<FString>& Args)
{
	APawn* Pawn = FUnrealcvServer::Get().GetPawn();
	if (IsValid(Pawn))
	{
		UVisionBPLib::UpdateInput(Pawn, true);
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error("The pawn is invalid");
	}
}

/** TODO: Update this with new API */
FExecStatus FActionHandler::SetStereoDistance(const TArray<FString>& Args)
{
	/*
	if (Args.Num() == 1) // Distance
	{
		int Distance = FCString::Atof(*Args[0]);
		UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(1); // This is the right eye camera
		CaptureComponent->SetRelativeLocation(FVector(0, Distance, 0));
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error("Expect argument: eye distance");
	}
	*/
	return FExecStatus::NotImplemented;
}

/** Return a TFunction to Release the Keyboard */
TFunction<void(void)> FActionHandler::GetReleaseKey(FKey Key)
{
	const UWorld* World = this->GetWorld();
	return [=]() {
		FInputKeyParams KeyParams(Key, EInputEvent::IE_Released, 0, false);
		World->GetFirstPlayerController()->InputKey(KeyParams);
	};
}

FExecStatus FActionHandler::Keyboard(const TArray<FString>& Args)
{
	if (Args.Num() != 2)
	{
		return FExecStatus::InvalidArgument;
	}
	FString KeyName = Args[0];

	UWorld* World = this->GetWorld();
	FKey Key(*KeyName);
	// The valid KeyName can be found in https://wiki.unrealengine.com/List_of_Key/Gamepad_Input_Names

	// Not sure about the meaning of parameters: DeltaTime, NumSamples, bGamepad
	// They are not clearly documented in
	// https://docs.unrealengine.com/latest/INT/API/Runtime/Engine/GameFramework/UPlayerInput/InputAxis/index.html
	// and https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Engine/Private/UserInterface/PlayerInput.cpp#L254
	float Delta = 1; // Delta is always 1 for key press, this is how hard this button is pressed
	float DeltaTime = FCString::Atof(*Args[1]);
	int32 NumSamples = 1;
	bool bGamepad = false;
	// The DeltaTime is not used in the code.
	FInputKeyParams KeyParams(Key, Delta, DeltaTime, NumSamples, bGamepad);
	World->GetFirstPlayerController()->InputKey(KeyParams);
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, GetReleaseKey(Key), DeltaTime, false);

	return FExecStatus::OK();
}

FExecStatus FActionHandler::GarbageCollection(const TArray<FString>& Args)
{
	GEngine->ForceGarbageCollection();
	return FExecStatus::OK();
}


FExecStatus FActionHandler::SetFixedFPS(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
	{
		return FExecStatus::Error("Missing parameter for frame rate");
	}
	float frame_rate = FCString::Atof(*Args[0]);
	GEngine->FixedFrameRate = frame_rate;
	GEngine->bUseFixedFrameRate = true;
	return FExecStatus::OK();
}
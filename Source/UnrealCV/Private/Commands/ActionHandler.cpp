#include "UnrealCVPrivate.h"
#include "ActionHandler.h"
#include "CaptureManager.h"

void FActionCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::PauseGame);
	Help = "Pause the game";
	CommandDispatcher->BindCommand("vset /action/game/pause", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::OpenLevel);
	Help = "Open level";
	CommandDispatcher->BindCommand("vset /action/game/level [str]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::EnableInput);
	Help = "Enable input";
	CommandDispatcher->BindCommand("vset /action/input/enable", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::DisableInput);
	Help = "Disable input";
	CommandDispatcher->BindCommand("vset /action/input/disable", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::SetStereoDistance);
	Help = "Set the distance of binocular stereo camera";
	CommandDispatcher->BindCommand("vset /action/eyes_distance [float]", Cmd, Help);

	
	/*
	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::ResumeGame);
	Help = "Resume the game";
	CommandDispatcher->BindCommand("vset /action/game/resume", Cmd, Help);
	*/

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::Keyboard);
	Help = "Send a keyboard action to the game";
	CommandDispatcher->BindCommand("vset /action/keyboard [str] [float]", Cmd, Help);
}

FExecStatus FActionCommandHandler::PauseGame(const TArray<FString>& Args)
{
	APlayerController* PlayerController = this->GetWorld()->GetFirstPlayerController();
	PlayerController->Pause();
	return FExecStatus::OK();
}

FExecStatus FActionCommandHandler::OpenLevel(const TArray<FString>& Args)
{
	if (Args.Num() == 1) // Level name
	{
		FString LevelName = Args[0];
		FUE4CVServer::Get().OpenLevel(FName(*LevelName));
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error("Expect argument: level name");
	}
}

FExecStatus FActionCommandHandler::EnableInput(const TArray<FString>& Args)
{
	FUE4CVServer::Get().UpdateInput(true);
	return FExecStatus::OK();
}

FExecStatus FActionCommandHandler::DisableInput(const TArray<FString>& Args)
{
	FUE4CVServer::Get().UpdateInput(false);
	return FExecStatus::OK();
}

FExecStatus FActionCommandHandler::SetStereoDistance(const TArray<FString>& Args)
{
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
}

/** Return a TFunction to Release the Keyboard */
TFunction<void(void)> FActionCommandHandler::GetReleaseKey(FKey Key)
{
	UWorld* World = this->GetWorld();
	return [=]() { 
		World->GetFirstPlayerController()->InputKey(Key, EInputEvent::IE_Released, 0, false);
	};
}

FExecStatus FActionCommandHandler::Keyboard(const TArray<FString>& Args)
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
	World->GetFirstPlayerController()->InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, GetReleaseKey(Key), DeltaTime, false);

	return FExecStatus::OK();
}
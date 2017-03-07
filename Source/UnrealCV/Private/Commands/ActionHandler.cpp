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

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::SetStereoDistance);
	Help = "Set the distance of binocular stereo camera";
	CommandDispatcher->BindCommand("vset /action/eyes_distance [float]", Cmd, Help);


	/*
	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::ResumeGame);
	Help = "Resume the game";
	CommandDispatcher->BindCommand("vset /action/game/resume", Cmd, Help);
	*/
}

FExecStatus FActionCommandHandler::PauseGame(const TArray<FString>& Args)
{
	APlayerController* PlayerController = this->GetWorld()->GetFirstPlayerController();
	PlayerController->Pause();
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

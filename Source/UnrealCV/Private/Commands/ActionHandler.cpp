#include "UnrealCVPrivate.h"
#include "ActionHandler.h"

void FActionCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FActionCommandHandler::PauseGame);
	Help = "Pause the game";
	CommandDispatcher->BindCommand("vset /action/game/pause", Cmd, Help);

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

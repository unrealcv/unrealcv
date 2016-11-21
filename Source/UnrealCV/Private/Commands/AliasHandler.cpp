#include "UnrealCVPrivate.h"
#include "AliasHandler.h"

void FAliasCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasCommandHandler::VRun);
	Help = "Run UE4 built-in commands";
	CommandDispatcher->BindCommand("vrun [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str] [str]", Cmd, Help);
}

FExecStatus FAliasCommandHandler::VRun(const TArray<FString>& Args)
{
	FString Cmd = "";
	
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; 
	check(GWorld->IsGameWorld());
	// GWorld->Exec(GWorld, *Cmd, *GetConsole());
	APlayerController* PlayerController = GWorld->GetFirstPlayerController();
	PlayerController->ConsoleCommand(Cmd, true);
	return FExecStatus::OK();
}

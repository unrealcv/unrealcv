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
	// vexec ActorId FuncName Params
	Help = "Run UE4 blueprint function";
	Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasCommandHandler::VExec);
	CommandDispatcher->BindCommand("vexec [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str] [str]", Cmd, Help);
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
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	check(World->IsGameWorld());

	APlayerController* PlayerController = World->GetFirstPlayerController();
	PlayerController->ConsoleCommand(Cmd, true);
	return FExecStatus::OK();
}

AActor* GetActorById(UWorld* World, FString ActorId)
{
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor->GetName() == ActorId)
		{
			return Actor;
		}
	}
	return nullptr;
}

UObject* GetObjectById(UWorld* World, FString ObjectId)
{
	for (TObjectIterator<UObject> ObjItr; ObjItr; ++ObjItr)
	{
		UObject* Obj = *ObjItr;
		if (Obj->GetName() == ObjectId)
		{
			return Obj;
		}
	}
	return nullptr;
}

FExecStatus FAliasCommandHandler::VExec(const TArray<FString>& Args)
{
	// Args[0] : ActorId
	// Args[1] : BlueprintFunctionName
	// Args[2 .. end] : Parameters

	FString ActorId, FuncName;
	if (Args.Num() < 1)
	{
		return FExecStatus::Error("The ActorId can not be empty.");
	}
	else
	{
		ActorId = Args[0];
	}
	
	if (Args.Num() < 2)
	{
		return FExecStatus::Error("The blueprint function name can not be empty.");
	}
	else
	{
		FuncName = Args[1];
	}

	UWorld* World;
	World = this->GetWorld();
	// AActor* Actor = GetActorById(World, ActorId);
	UObject* Obj = GetObjectById(World, ActorId);

	if (Obj == nullptr) return FExecStatus::Error(FString::Printf(TEXT("Can not find actor with id '%s'"), *ActorId));

	FString Cmd = FuncName;
	int ArgId = 2;
	while (ArgId < Args.Num())
	{
		Cmd += FString::Printf(TEXT(" %s"), *Args[ArgId]);
		ArgId++;
	}
	FOutputDeviceNull ar;

	// APlayerController* TestPlayerController = this->GetWorld()->GetFirstPlayerController();
	// FString ControllerName = TestPlayerController->GetName();
	// TestPlayerController->CallFunctionByNameWithArguments(TEXT("SetRotation 30 30 30"), ar, NULL, true);
	// check(TestPlayerController == Obj);
	// check(Cmd == TEXT("SetRotation 30 30 30"));

	// An example command is vexec RoboArmController_C_0 30 0 0
	if (Obj->CallFunctionByNameWithArguments(*Cmd, ar, nullptr, true))	
	{
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Fail to execute the function '%s' of %s"), *Cmd, *ActorId));
	}
}

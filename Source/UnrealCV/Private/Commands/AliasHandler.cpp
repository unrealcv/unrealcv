#include "AliasHandler.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/LevelScriptActor.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Core/Public/UObject/PropertyPortFlags.h"
#include "Utils/UObjectUtils.h"
#include "SerializeBPLib.h"
#include "UnrealcvLog.h"


void FAliasHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasHandler::VRun);
	Help = "Run UE4 built-in commands";
	CommandDispatcher->BindCommand("vrun [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vrun [str] [str] [str] [str] [str] [str]", Cmd, Help);

	// vexec ActorId FuncName Params
	Help = "Run UE4 blueprint function";
	Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasHandler::VExec);
	CommandDispatcher->BindCommand("vexec [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vexec [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasHandler::VExecWithOutput);
	CommandDispatcher->BindCommand("vbp [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
	CommandDispatcher->BindCommand("vbp [str] [str] [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);

	CommandDispatcher->BindCommand("vget /persistent_level/id",
		FDispatcherDelegate::CreateRaw(this, &FAliasHandler::GetPersistentLevelId),
		"Get persistent level id, so that we can call BP function defined in it"
		);

	CommandDispatcher->BindCommand("vget /persistent_level/level_script_actor/id",
		FDispatcherDelegate::CreateRaw(this, &FAliasHandler::GetLevelScriptActorId),
		"Get persistent level id, so that we can call BP function defined in it"
	);
}

FExecStatus FAliasHandler::VRun(const TArray<FString>& Args)
{
	FString Cmd = "";

	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1];
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	check(World->IsGameWorld());

	APlayerController* PlayerController = World->GetFirstPlayerController();
	PlayerController->ConsoleCommand(Cmd, true);
	return FExecStatus::OK();
}

FExecStatus FAliasHandler::VExecWithOutput(const TArray<FString>& Args)
{
	FString ActorId, FuncName;
	if (Args.Num() < 1) return FExecStatus::Error("The ActorId can not be empty.");

	ActorId = Args[0];

	if (Args.Num() < 2) return FExecStatus::Error("The blueprint function name can not be empty.");

	FuncName = Args[1];

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
	// Cmd = Cmd.TrimTrailing(); // TODO: Simplify this function.
	Cmd = Cmd.TrimEnd(); // New API

	FConsoleOutputDevice OutputDevice(FUnrealcvServer::Get().GetWorld()->GetGameViewport()->ViewportConsole);


	// if (Obj->CallFunctionByNameWithArguments(*Cmd, OutputDevice, nullptr, true))
	// {
	// 	return FExecStatus::OK();
	// }
	// else
	// {
	// 	return FExecStatus::Error(FString::Printf(TEXT("Fail to execute the function '%s' of %s"), *Cmd, *ActorId));
	// }

	const TCHAR* Str = *Cmd;
	FOutputDevice& Ar = OutputDevice;
	UObject* Executor = nullptr;
	bool bForceCallWithNonExec = true;/*=false*/

	// Find an exec function.
	FString MsgStr;
	if(!FParse::Token(Str,MsgStr,true))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not parse token"));
		return FExecStatus::InvalidArgument;
	}
	const FName Message = FName(*MsgStr,FNAME_Find);
	if(Message == NAME_None)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not find FName from token"));
		return FExecStatus::InvalidArgument;
	}
	UFunction* Function = Obj->FindFunction(Message);
	if(nullptr == Function)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not find function"));
		return FExecStatus::InvalidArgument;
	}
	if(0 == (Function->FunctionFlags & FUNC_Exec) && !bForceCallWithNonExec)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("BP function is not executable"));
		return FExecStatus::InvalidArgument;
	}

	UProperty* LastParameter = nullptr;

	// find the last parameter
	for ( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags&(CPF_Parm|CPF_ReturnParm)) == CPF_Parm; ++It )
	{
		LastParameter = *It;
	}

	// Parse all function parameters.
	uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
	FMemory::Memzero( Parms, Function->ParmsSize );

	for (TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		UProperty* LocalProp = *It;
		checkSlow(LocalProp);
		if (!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
		{
			LocalProp->InitializeValue_InContainer(Parms);
		}
	}

	const uint32 ExportFlags = PPF_None;
	bool bFailed = 0;
	int32 NumParamsEvaluated = 0;
	for( TFieldIterator<UProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm|CPF_ReturnParm))==CPF_Parm; ++It, NumParamsEvaluated++ )
	{
		if (It->HasAnyPropertyFlags(CPF_OutParm) || It->HasAnyPropertyFlags(CPF_ReferenceParm)) continue; // Skip return parameters.

		UProperty* PropertyParam = *It;
		checkSlow(PropertyParam); // Fix static analysis warning
		if (NumParamsEvaluated == 0 && Executor)
		{
			UObjectPropertyBase* Op = Cast<UObjectPropertyBase>(*It);
			if( Op && Executor->IsA(Op->PropertyClass) )
			{
				// First parameter is implicit reference to object executing the command.
				Op->SetObjectPropertyValue(Op->ContainerPtrToValuePtr<uint8>(Parms), Executor);
				continue;
			}
		}

		// Keep old string around in case we need to pass the whole remaining string
		const TCHAR* RemainingStr = Str;

		// Parse a new argument out of Str
		FString ArgStr;
		FParse::Token(Str, ArgStr, true);

		// if ArgStr is empty but we have more params to read parse the function to see if these have defaults, if so set them
		bool bFoundDefault = false;
		bool bFailedImport = true;
	#if WITH_EDITOR
		if (!FCString::Strcmp(*ArgStr, TEXT("")))
		{
			const FName DefaultPropertyKey(*(FString(TEXT("CPP_Default_")) + PropertyParam->GetName()));
			const FString& PropertyDefaultValue = Function->GetMetaData(DefaultPropertyKey);
			if (!PropertyDefaultValue.IsEmpty())
			{
				bFoundDefault = true;

				const TCHAR* Result = It->ImportText( *PropertyDefaultValue, It->ContainerPtrToValuePtr<uint8>(Parms), ExportFlags, NULL );
				bFailedImport = (Result == nullptr);
			}
		}
	#endif

		if (!bFoundDefault)
		{
			if (PropertyParam == LastParameter && PropertyParam->IsA<UStrProperty>() && FCString::Strcmp(Str, TEXT("")) != 0)
			{
				// if this is the last string property and we have remaining arguments to process, we have to assume that this
				// is a sub-command that will be passed to another exec (like "cheat giveall weapons", for example). Therefore
				// we need to use the whole remaining string as an argument, regardless of quotes, spaces etc.

				// ArgStr = FString(RemainingStr).Trim();
				ArgStr = FString(RemainingStr).TrimStart(); // New API
			}

			const TCHAR* Result = It->ImportText(*ArgStr, It->ContainerPtrToValuePtr<uint8>(Parms), ExportFlags, NULL );
			bFailedImport = (Result == nullptr);
		}

		if( bFailedImport )
		{
			FFormatNamedArguments Arguments;
			Arguments.Add(TEXT("Message"), FText::FromName( Message ));
			Arguments.Add(TEXT("PropertyName"), FText::FromName(It->GetFName()));
			Arguments.Add(TEXT("FunctionName"), FText::FromName(Function->GetFName()));
			Ar.Logf( TEXT("%s"), *FText::Format( NSLOCTEXT( "Core", "BadProperty", "'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}" ), Arguments ).ToString() );
			bFailed = true;

			break;
		}
	}

	if( !bFailed )
	{
		Obj->ProcessEvent( Function, Parms );
	}


	// https://answers.unrealengine.com/questions/139582/get-property-value.html
	// https://answers.unrealengine.com/questions/301428/set-string-for-fstring-via-uproperty.html
	// for (TFieldIterator<UIntProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_ReturnParm); ++It)
	for (TFieldIterator<UIntProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FString CPPType = It->GetCPPType();
		// float Target;
		// It->CopyCompleteValue_InContainer(&Dest, Parms);
		int Value = It->GetPropertyValue_InContainer(Parms);
	}

	// for (TFieldIterator<UNumericProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_ReturnParm); ++It)
	for (TFieldIterator<UNumericProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FString CPPType = It->GetCPPType();
		// float Target;
		// It->CopyCompleteValue_InContainer(&Dest, Parms);
		FString Value = It->GetNumericPropertyValueToString(Parms);
	}

	TMap<FString, FString> Dict;
	// CPF_OutParm, use this flag check!
	for (TFieldIterator<UStrProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		FString CPPType = It->GetCPPType();
		// float Target;
		// It->CopyCompleteValue_InContainer(&Dest, Parms);
		FString Value = It->GetPropertyValue_InContainer(Parms);
		// FString Key = It->GetName();
		// Dict.Emplace(Key, Value);
	}

	// Check SGraphNodeK2CreateDelegate.cpp
	for (TFieldIterator<UProperty> It(Function); It; ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_OutParm) || It->HasAnyPropertyFlags(CPF_ReferenceParm))
		{
			FString Key = It->GetName();
			FString Value;
			UStrProperty* StrProperty = Cast<UStrProperty>(*It);
			// if (IsValid(StrProperty))
			// {
			// 	Value = StrProperty->GetPropertyValue_InContainer(Parms);
			// }
			if (StrProperty != nullptr)
			{
				Value = StrProperty->GetPropertyValue_InContainer(Parms);
			}

			UNumericProperty* NumericProperty = Cast<UNumericProperty>(*It);
			// if (IsValid(NumericProperty))
			// {
			// Value = NumericProperty->GetNumericPropertyValueToString(Parms);
			// }
			if (NumericProperty != nullptr)
			{
				Value = NumericProperty->GetNumericPropertyValueToString(Parms);
			}

			if (StrProperty == nullptr && NumericProperty == nullptr)
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("Unrecognized type for parameter %s"), *Key);
			}
			
			Dict.Emplace(Key, Value);
		}
	}


	//!!destructframe see also UObject::ProcessEvent
	for( TFieldIterator<UProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It )
	{
		It->DestroyValue_InContainer(Parms);
	}

	FJsonObjectBP JsonObjectBP(Dict);

	// Success.
	return FExecStatus::OK(JsonObjectBP.ToString());

}

FExecStatus FAliasHandler::VExec(const TArray<FString>& Args)
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

	// FOutputDeviceNull OutputDevice;
	FConsoleOutputDevice OutputDevice(FUnrealcvServer::Get().GetWorld()->GetGameViewport()->ViewportConsole);
	// APlayerController* TestPlayerController = this->GetWorld()->GetFirstPlayerController();
	// FString ControllerName = TestPlayerController->GetName();
	// TestPlayerController->CallFunctionByNameWithArguments(TEXT("SetRotation 30 30 30"), ar, NULL, true);
	// check(TestPlayerController == Obj);
	// check(Cmd == TEXT("SetRotation 30 30 30"));

	// An example command is vexec RoboArmController_C_0 30 0 0
	check(Obj->IsPendingKillOrUnreachable() == false);
	EObjectFlags Flags = Obj->GetFlags();

	// From Actor.cpp ProcessEvent
	//#if WITH_EDITOR
	//static const FName CallInEditorMeta(TEXT("CallInEditor"));
	//const bool bAllowScriptExecution = GAllowActorScriptExecutionInEditor || Function->GetBoolMetaData(CallInEditorMeta);
	//#else
	//const bool bAllowScriptExecution = GAllowActorScriptExecutionInEditor;
	//#endif
	UWorld* MyWorld = Obj->GetWorld();
	//check((
	//		(MyWorld && (
	//					MyWorld->AreActorsInitialized()
	//					|| bAllowScriptExecution
	//					)
	//		)
	//		||
	//		Obj->HasAnyFlags(RF_ClassDefaultObject)
	//		)
	//		&&
	//		!Obj->IsGarbageCollecting()
	//	);
	if (MyWorld->AreActorsInitialized() == false)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Actors of the world are not initialized, the vexec might fail."));
	}

	if (Obj->CallFunctionByNameWithArguments(*Cmd, OutputDevice, nullptr, true))
	{
		return FExecStatus::OK();
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Fail to execute the function '%s' of %s"), *Cmd, *ActorId));
	}
}

FExecStatus FAliasHandler::GetPersistentLevelId(const TArray<FString>& Args)
{
	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
	
	if (IsValid(GameWorld) && IsValid(GameWorld->PersistentLevel))
	{
		return FExecStatus::OK(GameWorld->PersistentLevel->GetName());
	}
	else
	{
		return FExecStatus::Error("The UWorld is invalid");
	}
}

FExecStatus FAliasHandler::GetLevelScriptActorId(const TArray<FString>& Args)
{
	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();

	if (IsValid(GameWorld) && IsValid(GameWorld->PersistentLevel))
	{
		return FExecStatus::OK(GameWorld->PersistentLevel->LevelScriptActor->GetName());
	}
	else
	{
		return FExecStatus::Error("The UWorld is invalid");
	}
}

#include "AliasHandler.h"
#include "UnrealcvServer.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/Engine/Classes/Engine/LevelScriptActor.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Core/Public/UObject/PropertyPortFlags.h"
#include "Runtime/Core/Public/Async/AsyncWork.h"
#include "Runtime/Core/Public/Misc/OutputDeviceNull.h"
#include "Utils/UObjectUtils.h"
#include "Utils/ReflectionUtils.h"
#include "SerializeBPLib.h"
#include "UnrealcvLog.h"

// const static bool bUSE_ASYNC_NAV_TO_GOAL = true;
// const static bool bUSE_ASYNC_SET_MOVE = true;
const static TArray<FString> AsyncCommands = {
    TEXT("nav_to_goal"),
    TEXT("set_move"),
};

namespace
{

UObject* ResolveReflectionTarget(UWorld* World, const FString& TargetId, FString& OutError)
{
    if (TargetId.StartsWith(TEXT("class:"), ESearchCase::IgnoreCase) ||
        TargetId.StartsWith(TEXT("cdo:"), ESearchCase::IgnoreCase))
    {
        FString ClassNameOrPath = TargetId;
        int32 PrefixIndex = INDEX_NONE;
        if (ClassNameOrPath.FindChar(TEXT(':'), PrefixIndex))
        {
            ClassNameOrPath = ClassNameOrPath.Mid(PrefixIndex + 1);
        }

        UClass* Class = UnrealCV::ReflectionUtils::ResolveClass(ClassNameOrPath);
        if (Class == nullptr)
        {
            OutError = FString::Printf(TEXT("Can not find class '%s'"), *ClassNameOrPath);
            return nullptr;
        }

        UObject* DefaultObject = Class->GetDefaultObject();
        if (DefaultObject == nullptr)
        {
            OutError = FString::Printf(TEXT("Class '%s' has no default object"), *Class->GetName());
            return nullptr;
        }

        return DefaultObject;
    }

    UObject* Object = GetObjectById(World, TargetId);
    if (Object == nullptr)
    {
        OutError = FString::Printf(TEXT("Can not find object with id '%s'"), *TargetId);
    }
    return Object;
}

} // namespace

void FAliasHandler::RegisterCommands()
{
    FDispatcherDelegate Cmd;
    FString Help;

    Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasHandler::VRun);
    Help = "Run UE built-in commands";
    CommandDispatcher->BindCommand("vrun [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vrun [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vrun [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vrun [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vrun [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vrun [str] [str] [str] [str] [str] [str]", Cmd, Help);

    // vexec ActorId FuncName Params
    Help = "Run UE blueprint function";
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

    Help = "Reflection bridge for listing functions/properties and getting/setting properties";
    Cmd = FDispatcherDelegate::CreateRaw(this, &FAliasHandler::VReflect);
    CommandDispatcher->BindCommand("vreflect [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str] [str] [str] [str] [str]", Cmd,
                                   Help);
    CommandDispatcher->BindCommand("vreflect [str] [str] [str] [str] [str] [str] [str] [str] [str] [str] [str] [str]",
                                   Cmd, Help);
    CommandDispatcher->BindCommand("vreflect [str] call_json [str] [Anything]", Cmd, Help);

    CommandDispatcher->BindCommand("vget /persistent_level/id",
                                   FDispatcherDelegate::CreateRaw(this, &FAliasHandler::GetPersistentLevelId),
                                   "Get persistent level id, so that we can call BP function defined in it");

    CommandDispatcher->BindCommand("vget /persistent_level/level_script_actor/id",
                                   FDispatcherDelegate::CreateRaw(this, &FAliasHandler::GetLevelScriptActorId),
                                   "Get persistent level id, so that we can call BP function defined in it");
}

FExecStatus FAliasHandler::VReflect(const TArray<FString>& Args)
{
    if (Args.Num() < 2)
    {
        return FExecStatus::Error(TEXT("Usage: vreflect [object_id] [functions|properties|get|set] ..."));
    }

    const FString& ObjectId = Args[0];
    const FString Subcommand = Args[1].ToLower();

    FString ErrorMessage;
    UObject* Obj = ResolveReflectionTarget(this->GetWorld(), ObjectId, ErrorMessage);
    if (Obj == nullptr)
    {
        return FExecStatus::Error(ErrorMessage);
    }

    if (Subcommand == TEXT("functions"))
    {
        return FExecStatus::OK(UnrealCV::ReflectionUtils::SerializeFunctionsForObject(Obj).ToString());
    }

    if (Subcommand == TEXT("properties"))
    {
        return FExecStatus::OK(UnrealCV::ReflectionUtils::SerializePropertiesForObject(Obj).ToString());
    }

    if (Subcommand == TEXT("get"))
    {
        if (Args.Num() < 3)
        {
            return FExecStatus::Error(TEXT("Usage: vreflect [object_id] get [property_path]"));
        }

        void* ContainerPtr = nullptr;
        FProperty* Property = nullptr;
        if (!UnrealCV::ReflectionUtils::ResolvePropertyPath(Obj, Args[2], ContainerPtr, Property, ErrorMessage))
        {
            return FExecStatus::Error(ErrorMessage);
        }

        return FExecStatus::OK(UnrealCV::ReflectionUtils::SerializePropertyResult(Property, ContainerPtr).ToString());
    }

    if (Subcommand == TEXT("set"))
    {
        if (Args.Num() < 4)
        {
            return FExecStatus::Error(TEXT("Usage: vreflect [object_id] set [property_path] [value]"));
        }

        void* ContainerPtr = nullptr;
        FProperty* Property = nullptr;
        if (!UnrealCV::ReflectionUtils::ResolvePropertyPath(Obj, Args[2], ContainerPtr, Property, ErrorMessage))
        {
            return FExecStatus::Error(ErrorMessage);
        }

        FString ValueText = Args[3];
        for (int32 Index = 4; Index < Args.Num(); ++Index)
        {
            ValueText += TEXT(" ");
            ValueText += Args[Index];
        }

        if (!UnrealCV::ReflectionUtils::SetPropertyValueFromText(Property, ContainerPtr, ValueText, ErrorMessage))
        {
            return FExecStatus::Error(ErrorMessage);
        }

        return FExecStatus::OK(UnrealCV::ReflectionUtils::SerializePropertyResult(Property, ContainerPtr).ToString());
    }

    if (Subcommand == TEXT("call_json"))
    {
        if (Args.Num() < 4)
        {
            return FExecStatus::Error(
                TEXT("Usage: vreflect [object_id|class:class_name] call_json [function_name] [json_args]"));
        }

        FJsonObjectBP Result;
        if (!UnrealCV::ReflectionUtils::CallFunctionWithJson(Obj, Args[2], Args[3], Result, ErrorMessage))
        {
            return FExecStatus::Error(ErrorMessage);
        }

        return FExecStatus::OK(Result.ToString());
    }

    return FExecStatus::Error(FString::Printf(TEXT("Unknown vreflect subcommand '%s'"), *Args[1]));
}

FExecStatus FAliasHandler::VRun(const TArray<FString>& Args)
{
    FString Cmd = "";

    uint32 NumArgs = Args.Num();
    for (uint32 ArgIndex = 0; ArgIndex < NumArgs - 1; ArgIndex++)
    {
        Cmd += Args[ArgIndex] + " ";
    }
    Cmd += Args[NumArgs - 1];
    UWorld* World = FUnrealcvServer::Get().GetWorld();
    check(World->IsGameWorld());

    APlayerController* PlayerController = World->GetFirstPlayerController();
    PlayerController->ConsoleCommand(Cmd, true);
    return FExecStatus::OK();
}

FExecStatus FAliasHandler::VExecWithOutput(const TArray<FString>& Args)
{
    FString ActorId, FuncName;
    if (Args.Num() < 1)
        return FExecStatus::Error("The ActorId can not be empty.");

    ActorId = Args[0];

    if (Args.Num() < 2)
        return FExecStatus::Error("The blueprint function name can not be empty.");

    FuncName = Args[1];

    UWorld* World;
    World = this->GetWorld();
    // AActor* Actor = GetActorById(World, ActorId);
    UObject* Obj = GetObjectById(World, ActorId);

    if (Obj == nullptr)
        return FExecStatus::Error(FString::Printf(TEXT("Can not find actor with id '%s'"), *ActorId));

    for (const FString& AsyncCmd : AsyncCommands)
    {
        if (FuncName.Equals(AsyncCmd, ESearchCase::IgnoreCase))
        {
            FString CmdArgs = Args.Num() > 2 ? Args[2] : TEXT("");
            for (int32 ArgId = 3; ArgId < Args.Num(); ArgId++)
            {
                CmdArgs += FString::Printf(TEXT(" %s"), *Args[ArgId]);
            }
            HandleVBPAsync(Obj, FuncName, CmdArgs);
            UE_LOG(LogUnrealCV, Log, TEXT("VExecWithOutput: %s is async"), *FuncName);
            return FExecStatus::OK();
        }
    }

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
    bool bForceCallWithNonExec = true; /*=false*/

    // Find an exec function.
    FString MsgStr;
    if (!FParse::Token(Str, MsgStr, true))
    {
        UE_LOG(LogUnrealCV, Warning, TEXT("Can not parse token"));
        return FExecStatus::InvalidArgument;
    }
    const FName Message = FName(*MsgStr, FNAME_Find);
    if (Message == NAME_None)
    {
        UE_LOG(LogUnrealCV, Warning, TEXT("Can not find FName from token"));
        return FExecStatus::InvalidArgument;
    }
    UFunction* Function = Obj->FindFunction(Message);
    if (nullptr == Function)
    {
        UE_LOG(LogUnrealCV, Warning, TEXT("Can not find function"));
        return FExecStatus::InvalidArgument;
    }
    if (0 == (Function->FunctionFlags & FUNC_Exec) && !bForceCallWithNonExec)
    {
        UE_LOG(LogUnrealCV, Warning, TEXT("BP function is not executable"));
        return FExecStatus::InvalidArgument;
    }

    FProperty* LastParameter = nullptr;

    // find the last parameter
    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm;
         ++It)
    {
        LastParameter = *It;
    }

    // Parse all function parameters.
    uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
    FMemory::Memzero(Parms, Function->ParmsSize);

    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        FProperty* LocalProp = *It;
        checkSlow(LocalProp);
        if (!LocalProp->HasAnyPropertyFlags(CPF_ZeroConstructor))
        {
            LocalProp->InitializeValue_InContainer(Parms);
        }
    }

    const uint32 ExportFlags = PPF_None;
    bool bFailed = 0;
    int32 NumParamsEvaluated = 0;
    for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm;
         ++It, NumParamsEvaluated++)
    {
        if (It->HasAnyPropertyFlags(CPF_OutParm) || It->HasAnyPropertyFlags(CPF_ReferenceParm))
            continue; // Skip return parameters.

        FProperty* PropertyParam = *It;
        checkSlow(PropertyParam); // Fix static analysis warning
        if (NumParamsEvaluated == 0 && Executor)
        {
            FObjectPropertyBase* Op = CastField<FObjectPropertyBase>(*It);
            if (Op && Executor->IsA(Op->PropertyClass))
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

        // if ArgStr is empty but we have more params to read parse the function to see if these have defaults, if so
        // set them
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

                const TCHAR* Result = It->ImportText_Direct(
                    *PropertyDefaultValue, It->ContainerPtrToValuePtr<uint8>(Parms), nullptr, ExportFlags);
                bFailedImport = (Result == nullptr);
            }
        }
#endif

        if (!bFoundDefault)
        {
            if (PropertyParam == LastParameter && PropertyParam->IsA<FStrProperty>() &&
                FCString::Strcmp(Str, TEXT("")) != 0)
            {
                // if this is the last string property and we have remaining arguments to process, we have to assume
                // that this is a sub-command that will be passed to another exec (like "cheat giveall weapons", for
                // example). Therefore we need to use the whole remaining string as an argument, regardless of quotes,
                // spaces etc.

                // ArgStr = FString(RemainingStr).Trim();
                ArgStr = FString(RemainingStr).TrimStart(); // New API
            }

            const TCHAR* Result =
                It->ImportText_Direct(*ArgStr, It->ContainerPtrToValuePtr<uint8>(Parms), nullptr, ExportFlags);
            bFailedImport = (Result == nullptr);
        }

        if (bFailedImport)
        {
            FFormatNamedArguments Arguments;
            Arguments.Add(TEXT("Message"), FText::FromName(Message));
            Arguments.Add(TEXT("PropertyName"), FText::FromName(It->GetFName()));
            Arguments.Add(TEXT("FunctionName"), FText::FromName(Function->GetFName()));
            Ar.Logf(
                TEXT("%s"),
                *FText::Format(
                     NSLOCTEXT(
                         "Core", "BadProperty",
                         "'{Message}': Bad or missing property '{PropertyName}' when trying to call {FunctionName}"),
                     Arguments)
                     .ToString());
            bFailed = true;

            break;
        }
    }

    if (!bFailed)
    {
        Obj->ProcessEvent(Function, Parms);
    }

    FJsonObjectBP JsonObjectBP = UnrealCV::ReflectionUtils::SerializeOutputParameters(Function, Parms);

    //!!destructframe see also UObject::ProcessEvent
    for (TFieldIterator<FProperty> It(Function); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
    {
        It->DestroyValue_InContainer(Parms);
    }

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

    if (Obj == nullptr)
        return FExecStatus::Error(FString::Printf(TEXT("Can not find actor with id '%s'"), *ActorId));

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
    check(IsValid(Obj) && !Obj->IsUnreachable());
    EObjectFlags Flags = Obj->GetFlags();

    // From Actor.cpp ProcessEvent
    // #if WITH_EDITOR
    // static const FName CallInEditorMeta(TEXT("CallInEditor"));
    // const bool bAllowScriptExecution = GAllowActorScriptExecutionInEditor ||
    // Function->GetBoolMetaData(CallInEditorMeta); #else const bool bAllowScriptExecution =
    // GAllowActorScriptExecutionInEditor; #endif
    UWorld* MyWorld = Obj->GetWorld();
    // check((
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

void FAliasHandler::HandleVBPAsync(UObject* TargetObject, const FString& FuncName, const FString& Args)
{
    if (!IsValid(TargetObject))
    {
        UE_LOG(LogUnrealCV, Error, TEXT("HandleVBPAsync: Target object is invalid"));
        return;
    }

    UE_LOG(LogUnrealCV, Log, TEXT("HandleVBPAsync: Launch AsyncTask %s for %s with args %s"), *FuncName,
           *TargetObject->GetName(), *Args);
    AsyncTask(ENamedThreads::GameThread,
              [TargetObject, FuncName, Args]()
              {
                  if (!IsValid(TargetObject))
                  {
                      UE_LOG(LogUnrealCV, Error,
                             TEXT("HandleVBPAsync: Failed to execute %s for %s, reason: Target object is invalid, "
                                  "probably nullptr."),
                             *FuncName, *TargetObject->GetName());
                      return;
                  }
                  UE_LOG(LogUnrealCV, Log, TEXT("HandleVBPAsync: Start to execute %s for %s with args %s"), *FuncName,
                         *TargetObject->GetName(), *Args);

                  FConsoleOutputDevice OutputDevice(
                      FUnrealcvServer::Get().GetWorld()->GetGameViewport()->ViewportConsole);
                  FString Command = FuncName + TEXT(" ") + Args;

                  bool bSuccess = TargetObject->CallFunctionByNameWithArguments(*Command, OutputDevice, nullptr, true);

                  if (!bSuccess)
                  {
                      UE_LOG(LogUnrealCV, Error, TEXT("HandleVBPAsync: Failed to execute %s for %s"), *FuncName,
                             *TargetObject->GetName());
                  }
                  else
                  {
                      UE_LOG(LogUnrealCV, Log, TEXT("HandleVBPAsync: Successfully executed %s for %s"), *FuncName,
                             *TargetObject->GetName());
                  }
              });
}

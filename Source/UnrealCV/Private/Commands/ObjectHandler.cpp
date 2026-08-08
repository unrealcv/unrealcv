// Weichao Qiu @ 2017
#include "ObjectHandler.h"

#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

#include "UnrealcvServer.h"
#include "Controller/ActorController.h"
#include "VertexSensor.h"
#include "Utils/StrFormatter.h"
#include "Utils/UObjectUtils.h"
#include "VisionBPLib.h"
#include "CubeActor.h"
#include "CommandInterface.h"
#include "UnrealcvLog.h"
#include "BPFunctionLib/JsonObjectBP.h"
#include "BPFunctionLib/SerializeBPLib.h"

#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Runtime/Engine/Classes/Components/MeshComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"

namespace
{

FExecStatus SetActorName(AActor* Actor, const FString& NewName)
{
    UObject* ExistingObject = FindFirstObjectSafe<UObject>(*NewName);
    if (IsValid(ExistingObject))
    {
        if (ExistingObject == Actor)
        {
            return FExecStatus::OK(TEXT("The name has already been set"));
        }
        // IsValid succeeded so ExistingObject is guaranteed non-null.
        return FExecStatus::Error(
            FString::Printf(TEXT("Renaming an object to overwrite an existing object is not allowed: %s"), *NewName));
    }

    ICommandInterface* CmdActor = Cast<ICommandInterface>(Actor);
    if (CmdActor)
    {
        CmdActor->UnbindCommands();
    }
    Actor->Rename(*NewName);
    if (CmdActor)
    {
        CmdActor->BindCommands();
    }
    return FExecStatus::OK();
}

bool TryParseVector3(const FString& XArg, const FString& YArg, const FString& ZArg, FVector& OutVector)
{
    float X = 0.0f;
    float Y = 0.0f;
    float Z = 0.0f;
    if (!LexTryParseString(X, *XArg) || !LexTryParseString(Y, *YArg) || !LexTryParseString(Z, *ZArg))
    {
        return false;
    }

    OutVector = FVector(X, Y, Z);
    return true;
}

bool ParseOptionalSpawnNameAndLocation(const TArray<FString>& Args, int32 RequiredArgCount, FString& OutName,
                                       FVector& OutLocation, bool& bOutHasLocation)
{
    OutName.Reset();
    OutLocation = FVector::ZeroVector;
    bOutHasLocation = false;

    const int32 TrailingArgCount = Args.Num() - RequiredArgCount;
    if (TrailingArgCount == 0)
    {
        return true;
    }
    if (TrailingArgCount == 1)
    {
        OutName = Args[RequiredArgCount];
        return true;
    }
    if (TrailingArgCount == 3)
    {
        bOutHasLocation = TryParseVector3(Args[RequiredArgCount], Args[RequiredArgCount + 1],
                                          Args[RequiredArgCount + 2], OutLocation);
        return bOutHasLocation;
    }
    if (TrailingArgCount == 4)
    {
        OutName = Args[RequiredArgCount];
        bOutHasLocation = TryParseVector3(Args[RequiredArgCount + 1], Args[RequiredArgCount + 2],
                                          Args[RequiredArgCount + 3], OutLocation);
        return bOutHasLocation;
    }
    return false;
}

TArray<FName> ParseBoneNamesCsv(const FString& Csv)
{
    TArray<FString> BoneTokens;
    Csv.ParseIntoArray(BoneTokens, TEXT(","), true);

    TArray<FName> BoneNames;
    for (FString& BoneToken : BoneTokens)
    {
        BoneToken.TrimStartAndEndInline();
        if (!BoneToken.IsEmpty())
        {
            BoneNames.Add(FName(*BoneToken));
        }
    }
    return BoneNames;
}

} // anonymous namespace

void FObjectHandler::RegisterCommands()
{
    CommandDispatcher->BindCommand("vget /objects",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectList),
                                   "Get the name of all objects");

    CommandDispatcher->BindCommand("vset /objects/spawn_cube",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
                                   "Spawn a box in the scene for debugging purpose.");

    CommandDispatcher->BindCommand("vset /objects/spawn_cube [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
                                   "Spawn a box in the scene for debugging purpose, with optional argument name.");

    CommandDispatcher->BindCommand("vset /objects/spawn_cube [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
                                   "Spawn a box at world location [x, y, z].");

    CommandDispatcher->BindCommand("vset /objects/spawn_cube [str] [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
                                   "Spawn a named box at world location [x, y, z].");

    CommandDispatcher->BindCommand("vset /objects/spawn [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
                                   "Spawn an object with UClassName as the argument.");

    CommandDispatcher->BindCommand("vset /objects/spawn [str] [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
                                   "Spawn an object with UClassName as the argument.");

    CommandDispatcher->BindCommand("vset /objects/spawn [str] [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
                                   "Spawn an object at world location [x, y, z].");

    CommandDispatcher->BindCommand("vset /objects/spawn [str] [str] [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
                                   "Spawn a named object at world location [x, y, z].");

    CommandDispatcher->BindCommand("vget /object/[str]/location",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetLocation),
                                   "Get object location [x, y, z]");

    CommandDispatcher->BindCommand("vset /object/[str]/location [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetLocation),
                                   "Set object location [x, y, z]");

    CommandDispatcher->BindCommand("vget /object/[str]/rotation",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetRotation),
                                   "Get object rotation [pitch, yaw, roll]");

    CommandDispatcher->BindCommand("vset /object/[str]/rotation [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetRotation),
                                   "Set object rotation [pitch, yaw, roll]");

    CommandDispatcher->BindCommand("vget /object/[str]/vertex_location",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectVertexLocation),
                                   "Get vertex location");

    CommandDispatcher->BindCommand("vget /object/[str]/color",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetAnnotationColor),
                                   "Get the labeling color of an object (used in object instance mask)");

    CommandDispatcher->BindCommand("vset /object/[str]/color [uint] [uint] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetAnnotationColor),
                                   "Set the labeling color of an object [r, g, b]");

    CommandDispatcher->BindCommand("vget /object/[str]/mobility",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetMobility),
                                   "Is the object static or movable?");

    CommandDispatcher->BindCommand("vset /object/[str]/show",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetShow), "Show object");

    CommandDispatcher->BindCommand("vset /object/[str]/hide",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetHide), "Hide object");

    CommandDispatcher->BindCommand("vset /object/[str]/destroy",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Destroy), "Destroy object");

    CommandDispatcher->BindCommand("vget /object/[str]/uclass_name",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetUClassName),
                                   "Get the UClass name for filtering objects");

    CommandDispatcher->BindCommand("vset /object/[str]/name [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetName),
                                   "Set the name of the object");

#if WITH_EDITOR
    CommandDispatcher->BindCommand("vget /object/[str]/label",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetActorLabel),
                                   "Get actor label");

    CommandDispatcher->BindCommand("vset /object/[str]/label [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetActorLabel),
                                   "Set actor label");

    // Get the material(s) of an object and change it (first StaticMeshComponent).
    CommandDispatcher->BindCommand("vget /object/[str]/material",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetMaterial),
                                   "Get the material(s) of the object");

    CommandDispatcher->BindCommand("vset /object/[str]/material [uint] [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetMaterial),
                                   "Set material of the object (via slot index)");
#endif

    CommandDispatcher->BindCommand("vget /object/[str]/scale",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetScale),
                                   "Get object rotation [x, y, z]");

    CommandDispatcher->BindCommand("vset /object/[str]/scale [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetScale),
                                   "Set object scale [x, y, z]");

    CommandDispatcher->BindCommand("vget /object/[str]/bounds",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetBounds),
                                   "Return the bounds in the world coordinate, formate is [minx, y, z, maxx, y, z]");

    CommandDispatcher->BindCommand("vget /object/[str]/bones",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetBones),
                                   "Get all bone transforms in component space as JSON.");

    CommandDispatcher->BindCommand("vget /object/[str]/bones [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetBones),
                                   "Get comma-separated bones, or all bones in component|world space, as JSON.");

    CommandDispatcher->BindCommand("vget /object/[str]/bones [str] [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetBones),
                                   "Get comma-separated bones in component|world space as JSON.");
}

namespace
{

AActor* GetActor(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        return nullptr;
    }
    const FString& ActorId = Args[0];
    return GetActorById(FUnrealcvServer::Get().GetWorld(), ActorId);
}

#if WITH_EDITOR
UStaticMeshComponent* GetFirstStaticMeshComponent(AActor* Actor)
{
    if (!IsValid(Actor))
    {
        return nullptr;
    }
    TArray<UMeshComponent*> MeshComponents;
    Actor->GetComponents<UMeshComponent>(MeshComponents);
    for (UMeshComponent* MeshComponent : MeshComponents)
    {
        if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(MeshComponent))
        {
            return StaticMesh;
        }
    }
    return nullptr;
}
#endif

} // anonymous namespace

FExecStatus FObjectHandler::GetObjectList(const TArray<FString>& Args)
{
    TArray<AActor*> ActorList;
    UVisionBPLib::GetActorList(ActorList);

    FString StrActorList;
    for (AActor* Actor : ActorList)
    {
        StrActorList += FString::Printf(TEXT("%s "), *Actor->GetName());
    }
    return FExecStatus::OK(StrActorList);
}

FExecStatus FObjectHandler::GetLocation(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    FActorController Controller(Actor);
    FVector Location = Controller.GetLocation();

    FStrFormatter Ar;
    Ar << Location;

    return FExecStatus::OK(Ar.ToString());
}

/** There is no guarantee this will always succeed, for example, hitting a wall */
FExecStatus FObjectHandler::SetLocation(const TArray<FString>& Args)
{
    if (Args.Num() < 4)
    {
        return FExecStatus::InvalidArgument;
    }
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FActorController Controller(Actor);

    // TODO: Check whether this object is movable
    float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
    FVector NewLocation = FVector(X, Y, Z);
    Controller.SetLocation(NewLocation);

    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetRotation(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    FActorController Controller(Actor);
    FRotator Rotation = Controller.GetRotation();

    FStrFormatter Ar;
    Ar << Rotation;

    return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetRotation(const TArray<FString>& Args)
{
    if (Args.Num() < 4)
    {
        return FExecStatus::InvalidArgument;
    }
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FActorController Controller(Actor);

    // TODO: Check whether this object is movable
    float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
    FRotator Rotator = FRotator(Pitch, Yaw, Roll);
    Controller.SetRotation(Rotator);

    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetAnnotationColor(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FActorController Controller(Actor);

    FColor AnnotationColor;
    Controller.GetAnnotationColor(AnnotationColor);
    return FExecStatus::OK(AnnotationColor.ToString());
}

FExecStatus FObjectHandler::SetAnnotationColor(const TArray<FString>& Args)
{
    if (Args.Num() < 4)
    {
        return FExecStatus::InvalidArgument;
    }
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FActorController Controller(Actor);

    // ObjectName, R, G, B, A = 255
    // The color format is RGBA
    uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]),
           A = 255; // A = FCString::Atoi(*Args[4]);
    FColor AnnotationColor(R, G, B, A);

    Controller.SetAnnotationColor(AnnotationColor);
    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetMobility(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FActorController Controller(Actor);

    FString MobilityName = "";
    EComponentMobility::Type Mobility = Controller.GetMobility();
    switch (Mobility)
    {
    case EComponentMobility::Type::Movable:
        MobilityName = "Movable";
        break;
    case EComponentMobility::Type::Static:
        MobilityName = "Static";
        break;
    case EComponentMobility::Type::Stationary:
        MobilityName = "Stationary";
        break;
    default:
        MobilityName = "Unknown";
    }
    return FExecStatus::OK(MobilityName);
}

FExecStatus FObjectHandler::SetShow(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    FActorController Controller(Actor);
    Controller.Show();
    return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetHide(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    FActorController Controller(Actor);
    Controller.Hide();
    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetObjectVertexLocation(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));
    FVertexSensor VertexSensor(Actor);
    TArray<FVector> VertexArray = VertexSensor.GetVertexArray();

    // Serialize it to json?
    FString Str = "";
    for (const auto& Vertex : VertexArray)
    {
        FString VertexLocation = FString::Printf(TEXT("%.5f     %.5f     %.5f"), Vertex.X, Vertex.Y, Vertex.Z);
        Str += VertexLocation + "\n";
    }

    return FExecStatus::OK(Str);
}

/** A debug utility function to create StaticBox through python API */
FExecStatus FObjectHandler::SpawnBox(const TArray<FString>& Args)
{
    FString ObjectName;
    FVector SpawnLocation = FVector::ZeroVector;
    bool bHasSpawnLocation = false;
    if (!ParseOptionalSpawnNameAndLocation(Args, 0, ObjectName, SpawnLocation, bHasSpawnLocation))
    {
        return FExecStatus::InvalidArgument;
    }

    UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
    if (!IsValid(GameWorld))
    {
        return FExecStatus::Error(TEXT("Cannot get world"));
    }

    FActorSpawnParameters SpawnParameters;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = GameWorld->SpawnActor<ACubeActor>(ACubeActor::StaticClass(),
                                                      bHasSpawnLocation ? SpawnLocation : FVector::ZeroVector,
                                                      FRotator::ZeroRotator, SpawnParameters);
    if (!IsValid(Actor))
    {
        return FExecStatus::Error(TEXT("Failed to spawn cube actor"));
    }
    if (!ObjectName.IsEmpty())
    {
        const FExecStatus RenameStatus = SetActorName(Actor, ObjectName);
        if (RenameStatus.GetStatusType() == EExecStatusType::Error)
        {
            Actor->Destroy();
            return RenameStatus;
        }
    }
    return FExecStatus::OK(Actor->GetName());
}

FExecStatus FObjectHandler::Spawn(const TArray<FString>& Args)
{
    if (Args.Num() < 1 || Args.Num() > 5)
    {
        return FExecStatus::InvalidArgument;
    }

    FString ActorName;
    FVector SpawnLocation = FVector::ZeroVector;
    bool bHasSpawnLocation = false;
    if (!ParseOptionalSpawnNameAndLocation(Args, 1, ActorName, SpawnLocation, bHasSpawnLocation))
    {
        return FExecStatus::InvalidArgument;
    }

    if (!ActorName.IsEmpty())
    {
        AActor* Actor = GetActorById(FUnrealcvServer::Get().GetWorld(), ActorName);
        if (IsValid(Actor))
        {
            FString ErrorMsg = FString::Printf(TEXT("Failed to spawn %s, object exists."), *ActorName);
            UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ErrorMsg);
            return FExecStatus::Error(ErrorMsg);
        }
    }

    const FString& UClassName = Args[0];
    // Lookup UClass with a string
    UClass* Class = FindFirstObjectSafe<UClass>(*UClassName);

    if (!IsValid(Class))
    {
        FString ErrorMsg = FString::Printf(TEXT("Can not find a class with name '%s'"), *UClassName);
        UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ErrorMsg);
        return FExecStatus::Error(ErrorMsg);
    }

    UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
    if (!IsValid(GameWorld))
    {
        return FExecStatus::Error(TEXT("Cannot get world"));
    }
    FActorSpawnParameters SpawnParameters;
    // SpawnParameters.bNoFail = true; // Allow collision during spawn.
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = GameWorld->SpawnActor<AActor>(Class, bHasSpawnLocation ? SpawnLocation : FVector::ZeroVector,
                                                  FRotator::ZeroRotator, SpawnParameters);
    if (!IsValid(Actor))
    {
        return FExecStatus::Error(TEXT("Failed to spawn actor"));
    }

    if (!ActorName.IsEmpty())
    {
        const FExecStatus RenameStatus = SetActorName(Actor, ActorName);
        if (RenameStatus.GetStatusType() == EExecStatusType::Error)
        {
            Actor->Destroy();
            return RenameStatus;
        }
    }
    return FExecStatus::OK(Actor->GetName());
}

FExecStatus FObjectHandler::GetBones(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
    {
        return FExecStatus::Error(TEXT("Cannot find object"));
    }

    TArray<FName> BoneNames;
    bool bWorldSpace = false;
    if (Args.Num() >= 2)
    {
        if (Args[1].Equals(TEXT("world"), ESearchCase::IgnoreCase) ||
            Args[1].Equals(TEXT("component"), ESearchCase::IgnoreCase))
        {
            bWorldSpace = Args[1].Equals(TEXT("world"), ESearchCase::IgnoreCase);
        }
        else
        {
            BoneNames = ParseBoneNamesCsv(Args[1]);
        }
    }
    if (Args.Num() >= 3)
    {
        if (!Args[2].Equals(TEXT("world"), ESearchCase::IgnoreCase) &&
            !Args[2].Equals(TEXT("component"), ESearchCase::IgnoreCase))
        {
            return FExecStatus::Error(TEXT("Bone space must be component or world"));
        }
        bWorldSpace = Args[2].Equals(TEXT("world"), ESearchCase::IgnoreCase);
    }

    TArray<UPoseableMeshComponent*> PoseableMeshComponents;
    Actor->GetComponents<UPoseableMeshComponent>(PoseableMeshComponents);
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

    USkinnedMeshComponent* SkinnedMeshComponent = nullptr;
    if (PoseableMeshComponents.Num() > 0 && IsValid(PoseableMeshComponents[0]))
    {
        SkinnedMeshComponent = PoseableMeshComponents[0];
    }
    else if (SkeletalMeshComponents.Num() > 0 && IsValid(SkeletalMeshComponents[0]))
    {
        SkinnedMeshComponent = SkeletalMeshComponents[0];
    }
    if (!IsValid(SkinnedMeshComponent))
    {
        return FExecStatus::Error(TEXT("No skeletal or poseable mesh component found"));
    }

    if (BoneNames.IsEmpty())
    {
        const int32 NumBones = SkinnedMeshComponent->GetNumBones();
        BoneNames.Reserve(NumBones);
        for (int32 BoneIndex = 0; BoneIndex < NumBones; ++BoneIndex)
        {
            BoneNames.Add(SkinnedMeshComponent->GetBoneName(BoneIndex));
        }
    }

    UPoseableMeshComponent* PoseableMeshComponent = Cast<UPoseableMeshComponent>(SkinnedMeshComponent);
    USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(SkinnedMeshComponent);
    TArray<FJsonObjectBP> BoneJsonArray;
    BoneJsonArray.Reserve(BoneNames.Num());
    for (const FName& BoneName : BoneNames)
    {
        const int32 BoneIndex = SkinnedMeshComponent->GetBoneIndex(BoneName);
        if (BoneIndex == INDEX_NONE)
        {
            continue;
        }

        FTransform BoneTransform = FTransform::Identity;
        if (PoseableMeshComponent)
        {
            BoneTransform = PoseableMeshComponent->GetBoneTransformByName(
                BoneName, bWorldSpace ? EBoneSpaces::WorldSpace : EBoneSpaces::ComponentSpace);
        }
        else if (SkeletalMeshComponent)
        {
            BoneTransform = SkeletalMeshComponent->GetBoneTransform(BoneIndex);
            if (!bWorldSpace)
            {
                BoneTransform = BoneTransform.GetRelativeTransform(SkeletalMeshComponent->GetComponentTransform());
            }
        }

        const TArray<FString> Keys = {TEXT("bone_name"), TEXT("transform")};
        const TArray<FJsonObjectBP> Values = {FJsonObjectBP(BoneName.ToString()), FJsonObjectBP(BoneTransform)};
        BoneJsonArray.Add(USerializeBPLib::TMapToJson(Keys, Values));
    }

    return FExecStatus::OK(FJsonObjectBP(BoneJsonArray).ToString());
}

FExecStatus FObjectHandler::Destroy(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    Actor->Destroy();
    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetUClassName(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    FString UClassName = Actor->GetClass()->GetName();
    return FExecStatus::OK(UClassName);
}

/** Get component names of the object */
FExecStatus FObjectHandler::GetComponents(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
    {
        return FExecStatus::Error(TEXT("Cannot find object"));
    }

    FString Result;
    TInlineComponentArray<UActorComponent*> Components;
    Actor->GetComponents(Components);
    for (const UActorComponent* Component : Components)
    {
        if (IsValid(Component))
        {
            Result += Component->GetName() + TEXT(" ");
        }
    }
    return FExecStatus::OK(Result);
}

FExecStatus FObjectHandler::SetName(const TArray<FString>& Args)
{
    if (Args.Num() != 2)
    {
        return FExecStatus::InvalidArgument;
    }
    AActor* Actor = GetActor(Args);
    if (!Actor)
        return FExecStatus::Error(TEXT("Cannot find object"));

    const FString& NewName = Args[1];
    // Check whether the object name already exists, otherwise it will crash in
    // File:/home/qiuwch/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/Obj.cpp] [Line: 198]
    FExecStatus Status = SetActorName(Actor, NewName);

    return Status;
}

#if WITH_EDITOR
FExecStatus FObjectHandler::SetActorLabel(const TArray<FString>& Args)
{
    FString ActorLabel;
    if (Args.Num() == 2)
    {
        ActorLabel = Args[1];
    }

    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
        return FExecStatus::Error(TEXT("Cannot find object"));

    Actor->SetActorLabel(ActorLabel);
    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetActorLabel(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
        return FExecStatus::Error(TEXT("Cannot find object"));

    FString ActorLabel = Actor->GetActorLabel();
    return FExecStatus::OK(ActorLabel);
}

FExecStatus FObjectHandler::GetMaterial(const TArray<FString>& Args)
{
    AActor* TargetActor = GetActor(Args);
    if (!IsValid(TargetActor))
    {
        return FExecStatus::Error(TEXT("Can not find object"));
    }

    UStaticMeshComponent* MeshComponent = GetFirstStaticMeshComponent(TargetActor);
    if (!MeshComponent)
    {
        return FExecStatus::Error(TEXT("Object has no StaticMeshComponent"));
    }

    FString MaterialInfo;

    for (int32 SlotIndex = 0; SlotIndex < MeshComponent->GetNumMaterials(); ++SlotIndex)
    {
        if (UMaterialInterface* Material = MeshComponent->GetMaterial(SlotIndex))
        {
            MaterialInfo += FString::Printf(TEXT("[%d] %s "), SlotIndex, *Material->GetPathName());
        }
    }

    return FExecStatus::OK(MaterialInfo);
}

FExecStatus FObjectHandler::SetMaterial(const TArray<FString>& Args)
{
    if (Args.Num() != 3)
    {
        return FExecStatus::InvalidArgument;
    }

    AActor* TargetActor = GetActor(Args);
    if (!IsValid(TargetActor))
    {
        return FExecStatus::Error(TEXT("Can not find object"));
    }

    const int32 SlotIndex = FCString::Atoi(*Args[1]);
    const FString& MaterialPath = Args[2];

    UStaticMeshComponent* MeshComponent = GetFirstStaticMeshComponent(TargetActor);
    if (!MeshComponent)
    {
        return FExecStatus::Error(TEXT("Object has no StaticMeshComponent"));
    }

    if (SlotIndex < 0 || SlotIndex >= MeshComponent->GetNumMaterials())
    {
        return FExecStatus::Error(TEXT("Invalid material slot"));
    }

    UMaterialInterface* NewMaterial =
        Cast<UMaterialInterface>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));

    if (!NewMaterial)
    {
        return FExecStatus::Error(TEXT("Can not load material"));
    }

    MeshComponent->SetMaterial(SlotIndex, NewMaterial);
    return FExecStatus::OK();
}
#endif

FExecStatus FObjectHandler::GetScale(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
        return FExecStatus::Error(TEXT("Cannot find object"));

    FVector Scale = Actor->GetActorScale3D();

    FStrFormatter Ar;
    Ar << Scale;

    return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetScale(const TArray<FString>& Args)
{
    if (Args.Num() < 4)
    {
        return FExecStatus::InvalidArgument;
    }
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
        return FExecStatus::Error(TEXT("Cannot find object"));

    float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
    FVector Scale = FVector(X, Y, Z);
    Actor->SetActorScale3D(Scale);

    return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetBounds(const TArray<FString>& Args)
{
    AActor* Actor = GetActor(Args);
    if (!IsValid(Actor))
        return FExecStatus::Error(TEXT("Cannot find object"));

    bool bOnlyCollidingComponents = false;
    FVector Origin, BoundsExtent;
    Actor->GetActorBounds(bOnlyCollidingComponents, Origin, BoundsExtent);
    FVector Min = Origin - BoundsExtent, Max = Origin + BoundsExtent;

    FString Res = FString::Printf(TEXT("%.2f %.2f %.2f %.2f %.2f %.2f"), Min.X, Min.Y, Min.Z, Max.X, Max.Y, Max.Z);

    return FExecStatus::OK(Res);
}

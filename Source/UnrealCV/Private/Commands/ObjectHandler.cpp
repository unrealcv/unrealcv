// Weichao Qiu @ 2017
#include "ObjectHandler.h"

#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Runtime/CoreUObject/Public/UObject/Package.h"

#include "UnrealcvServer.h"
#include "Controller/ActorController.h"
#include "VertexSensor.h"
#include "Utils/StrFormatter.h"
#include "Utils/UObjectUtils.h"
#include "VisionBPLib.h"
#include "CubeActor.h"
#include "CommandInterface.h"

FExecStatus SetActorName(AActor* Actor, FString NewName)
{
	// UObject* NameScopeOuter = ANY_PACKAGE;
	// UObject* ExistingObject = StaticFindObject(/*Class=*/ NULL, NameScopeOuter, *NewName, true);
	UObject* ExistingObject = FindFirstObjectSafe<UObject>(*NewName);
	if (IsValid(ExistingObject))
	{
		if (ExistingObject == Actor)
		{
			FString ErrorMsg = TEXT("The name has already been set");
			return FExecStatus::OK(ErrorMsg);
		}
		else if (ExistingObject)
		{
			FString ErrorMsg = FString::Printf(TEXT("Renaming an object to overwrite an existing object is not allowed, %s"),  *NewName);
			return FExecStatus::Error(ErrorMsg);
		}
	}
	else
	{
		ICommandInterface* CmdActor = Cast<ICommandInterface>(Actor);
		if (CmdActor) CmdActor->UnbindCommands();
		Actor->Rename(*NewName);
		if (CmdActor) CmdActor->BindCommands();
	}
	return FExecStatus::OK();
}

void FObjectHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /objects",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectList),
		"Get the name of all objects"
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn_cube",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
		"Spawn a box in the scene for debugging purpose."
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn_bp_asset [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBpAsset),
		"Spawn a blueprint asset in content"
	);


	CommandDispatcher->BindCommand(
		"vset /objects/spawn_bp_asset [str] [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBpAsset),
		"Spawn a blueprint asset in content"
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn_cube [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SpawnBox),
		"Spawn a box in the scene for debugging purpose, with optional argument name."
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
		"Spawn an object with UClassName as the argument."
	);

	CommandDispatcher->BindCommand(
		"vset /objects/spawn [str] [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Spawn),
		"Spawn an object with UClassName as the argument."
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetLocation),
		"Get object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/location [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetLocation),
		"Set object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/rotation",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetRotation),
		"Get object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetRotation),
		"Set object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/vertex_location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectVertexLocation),
		"Get vertex location"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/color",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetAnnotationColor),
		"Get the labeling color of an object (used in object instance mask)"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/color [uint] [uint] [uint]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetAnnotationColor),
		"Set the labeling color of an object [r, g, b]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/mobility",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetMobility),
		"Is the object static or movable?"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/show",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetShow),
		"Show object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/hide",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetHide),
		"Hide object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/destroy",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::Destroy),
		"Destroy object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/physics [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetPhysics),
		"Set the physics of the object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/collision [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetCollision),
		"Set the collision of the object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/object_mobility [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetObjectMobility),
		"Set the mobility of the object"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/uclass_name",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetUClassName),
		"Get the UClass name for filtering objects"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/name [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetName),
		"Set the name of the object"
	);

#if WITH_EDITOR
	CommandDispatcher->BindCommand(
		"vget /object/[str]/label",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetActorLabel),
		"Get actor label"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/label [str]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetActorLabel),
		"Set actor label"
	);
#endif

	CommandDispatcher->BindCommand(
		"vget /object/[str]/scale",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetScale),
		"Get object rotation [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/scale [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetScale),
		"Set object scale [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/bounds",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetBounds),
		"Return the bounds in the world coordinate, formate is [minx, y, z, maxx, y, z]"
	);
}

AActor* GetActor(const TArray<FString>& Args)
{
	FString ActorId = Args[0];
	AActor* Actor = GetActorById(FUnrealcvServer::Get().GetWorld(), ActorId);
	return Actor;
}

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
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FVector Location = Controller.GetLocation();

	FStrFormatter Ar;
	Ar << Location;

	return FExecStatus::OK(Ar.ToString());
}

/** There is no guarantee this will always succeed, for example, hitting a wall */
FExecStatus FObjectHandler::SetLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
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
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FRotator Rotation = Controller.GetRotation();

	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
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
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FColor AnnotationColor;
	Controller.GetAnnotationColor(AnnotationColor);
	return FExecStatus::OK(AnnotationColor.ToString());
}

FExecStatus FObjectHandler::SetAnnotationColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	// ObjectName, R, G, B, A = 255
	// The color format is RGBA
	uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = 255; // A = FCString::Atoi(*Args[4]);
	FColor AnnotationColor(R, G, B, A);

	Controller.SetAnnotationColor(AnnotationColor);
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetMobility(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FString MobilityName = "";
	EComponentMobility::Type Mobility = Controller.GetMobility();
	switch (Mobility)
	{
		case EComponentMobility::Type::Movable: MobilityName = "Movable"; break;
		case EComponentMobility::Type::Static: MobilityName = "Static"; break;
		case EComponentMobility::Type::Stationary: MobilityName = "Stationary"; break;
		default: MobilityName = "Unknown";
	}
	return FExecStatus::OK(MobilityName);
}

FExecStatus FObjectHandler::SetShow(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Show();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SpawnBpAsset(const TArray<FString>& Args)
{
	if (Args.Num() == 2)
	{
		FString ActorId = Args[1];
		AActor* Actor = GetActorById(FUnrealcvServer::Get().GetWorld(), ActorId);
		if (IsValid(Actor))
		{
			FString ErrorMsg = FString::Printf(TEXT("Failed to spawn %s, object exsit."), *ActorId);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
			return FExecStatus::Error(ErrorMsg);
		}
	}
	
	FString BlueprintPath;
	if (Args.Num() == 1 || Args.Num() == 2)
	{
		BlueprintPath = Args[0];
	}
	//TEXT("Blueprint'/Game/CityDatabase/blueprints/BP_Building_01.BP_Building_01_C'")

	UClass* BlueprintClass = LoadClass<AActor>(nullptr, *BlueprintPath);
	if (BlueprintClass == nullptr)
	{
		FString ErrorMsg = FString::Printf(TEXT("Can not find the blueprint '%s'"), *BlueprintPath);
		//FString ErrorMsg = FString::Printf(TEXT("Can not find the blueprint"));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
		return FExecStatus::Error(ErrorMsg);
	}

	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
	FActorSpawnParameters SpawnParameters;
	// SpawnParameters.bNoFail = true; // Allow collision during spawn.
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Actor = GameWorld->SpawnActor(BlueprintClass, NULL, NULL, SpawnParameters);
	if (!IsValid(Actor))
	{
		return FExecStatus::Error("Failed to spawn actor");
	}

	if (Args.Num() == 2)
	{
		FString ActorName = Args[1];
		SetActorName(Actor, ActorName);
	}
	return FExecStatus::OK(Actor->GetName());
}


FExecStatus FObjectHandler::SetHide(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Hide();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetObjectVertexLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FVertexSensor VertexSensor(Actor);
	TArray<FVector> VertexArray = VertexSensor.GetVertexArray();

	// Serialize it to json?
	FString Str = "";
	for (auto Vertex : VertexArray)
	{
		FString VertexLocation = FString::Printf(
			TEXT("%.5f     %.5f     %.5f"),
			Vertex.X, Vertex.Y, Vertex.Z);
		Str += VertexLocation + "\n";
	}

	return FExecStatus::OK(Str);
}

/** A debug utility function to create StaticBox through python API */
FExecStatus FObjectHandler::SpawnBox(const TArray<FString>& Args)
{
	FString ObjectName;
	if (Args.Num() == 1) { ObjectName = Args[0]; }

	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
	AActor* Actor = GameWorld->SpawnActor(ACubeActor::StaticClass());

	return FExecStatus::OK();
}

FExecStatus FObjectHandler::Spawn(const TArray<FString>& Args)
{
	if (Args.Num() == 2)
	{
		FString ActorId = Args[1];
		AActor* Actor = GetActorById(FUnrealcvServer::Get().GetWorld(), ActorId);
		if (IsValid(Actor))
		{
			FString ErrorMsg = FString::Printf(TEXT("Failed to spawn %s, object exsit."), *ActorId);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
			return FExecStatus::Error(ErrorMsg);
		}
	}

	FString UClassName;
	if (Args.Num() == 1 || Args.Num() == 2)
	{ 
		UClassName = Args[0]; 
	}
	// Lookup UClass with a string
	UClass*	Class = FindFirstObjectSafe<UClass>(*UClassName);

	if (!IsValid(Class))
	{
		FString ErrorMsg = FString::Printf(TEXT("Can not find a class with name '%s'"), *UClassName);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *ErrorMsg);
		return FExecStatus::Error(ErrorMsg);
	}

	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
	FActorSpawnParameters SpawnParameters;
	// SpawnParameters.bNoFail = true; // Allow collision during spawn.
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Actor = GameWorld->SpawnActor(Class, NULL, NULL, SpawnParameters);
	if (!IsValid(Actor))
	{
		return FExecStatus::Error("Failed to spawn actor");
	}

	if (Args.Num() == 2)
	{
		FString ActorName = Args[1];
		SetActorName(Actor, ActorName);
	}
	return FExecStatus::OK(Actor->GetName());
}

FExecStatus FObjectHandler::Destroy(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	Actor->Destroy();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetPhysics(const TArray<FString>& Args)
{
	FString EnablePhysics;
	if (Args.Num() < 1) return FExecStatus::Error("The ActorId can not be empty.");
	AActor* Actor = GetActor(Args);

	if (Args.Num() < 2) return FExecStatus::Error("Need second parameter to define if this object has physics.");
	EnablePhysics = Args[1].ToLower();

	UPrimitiveComponent* component = Cast<UPrimitiveComponent>(Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));

	if (EnablePhysics == "true") {
		component->SetSimulatePhysics(true);
	}
	else if (EnablePhysics == "false") {
		component->SetSimulatePhysics(false);
	}
	else {
		return FExecStatus::Error("Please input true or false in the second parameter.");
	}
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetCollision(const TArray<FString>& Args)
{
	FString EnablePhysics;
	if (Args.Num() < 1) return FExecStatus::Error("The ActorId can not be empty.");
	AActor* Actor = GetActor(Args);

	if (Args.Num() < 2) return FExecStatus::Error("Need second parameter to define if this object has collision.");
	EnablePhysics = Args[1].ToLower();

	UPrimitiveComponent* component = Cast<UPrimitiveComponent>(Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));

	if (EnablePhysics == "true") {
		component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else if (EnablePhysics == "false") {
		component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else {
		return FExecStatus::Error("Please input true or false in the second parameter.");
	}
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetObjectMobility(const TArray<FString>& Args)
{
	FString EnableMobility;
	if (Args.Num() < 1) return FExecStatus::Error("The ActorId can not be empty.");
	AActor* Actor = GetActor(Args);

	if (Args.Num() < 2) return FExecStatus::Error("Need second parameter to define if this object has mobility.");
	EnableMobility = Args[1].ToLower();

	UPrimitiveComponent* component = Cast<UPrimitiveComponent>(Actor->GetComponentByClass(UPrimitiveComponent::StaticClass()));

	if (EnableMobility == "true") {
		component->SetMobility(EComponentMobility::Movable);
	}
	else if (EnableMobility == "false") {
		component->SetMobility(EComponentMobility::Stationary);
	}
	else {
		return FExecStatus::Error("Please input true or false in the second parameter.");
	}
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetUClassName(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FString UClassName = Actor->StaticClass()->GetName();
	return FExecStatus::OK(UClassName);
}

/** Get component names of the object */
FExecStatus FObjectHandler::GetComponents(const TArray<FString>& Args)
{
	return FExecStatus::OK();
}


FExecStatus FObjectHandler::SetName(const TArray<FString>& Args)
{
	if (Args.Num() != 2)
	{
		return FExecStatus::InvalidArgument;
	}
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FString NewName = Args[1];
	// Check whether the object name already exists, otherwise it will crash in 
	// File:/home/qiuwch/UnrealEngine/Engine/Source/Runtime/CoreUObject/Private/UObject/Obj.cpp] [Line: 198]
	FExecStatus Status = SetActorName(Actor, NewName);

	return Status; 
}

#if WITH_EDITOR
FExecStatus FObjectHandler::SetActorLabel(const TArray<FString>& Args)
{
	FString ActorLabel;
	if (Args.Num() == 2) { ActorLabel = Args[1]; }

	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	Actor->SetActorLabel(ActorLabel);
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetActorLabel(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	FString ActorLabel = Actor->GetActorLabel();
	return FExecStatus::OK(ActorLabel);
}
#endif 


FExecStatus FObjectHandler::GetScale(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	FVector Scale = Actor->GetActorScale3D();

	FStrFormatter Ar;
	Ar << Scale;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetScale(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
	FVector Scale = FVector(X, Y, Z);
	Actor->SetActorScale3D(Scale);

	return FExecStatus::OK();
}

FExecStatus FObjectHandler::GetBounds(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!IsValid(Actor)) return FExecStatus::Error("Can not find object");

	bool bOnlyCollidingComponents = false;
	FVector Origin, BoundsExtent;
	Actor->GetActorBounds(bOnlyCollidingComponents, Origin, BoundsExtent);  
	FVector Min = Origin - BoundsExtent, Max = Origin + BoundsExtent;

	FString Res = FString::Printf(TEXT("%.2f %.2f %.2f %.2f %.2f %.2f"), 
		Min.X, Min.Y, Min.Z, Max.X, Max.Y, Max.Z);

	return FExecStatus::OK(Res);
}



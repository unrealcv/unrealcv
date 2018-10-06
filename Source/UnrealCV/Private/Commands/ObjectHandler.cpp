// Weichao Qiu @ 2017
#include "ObjectHandler.h"
#include "Controller/ActorController.h"
#include "VertexSensor.h"
#include "Utils/StrFormatter.h"
#include "Utils/UObjectUtils.h"
#include "VisionBPLib.h"

void FObjectHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /objects",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectList),
		"Get the name of all objects"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectLocation),
		"Get object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/location [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetObjectLocation),
		"Set object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/rotation",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectRotation),
		"Get object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetObjectRotation),
		"Set object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/vertex_location",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectVertexLocation),
		"Get vertex location"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/color",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectAnnotationColor),
		"Get the labeling color of an object (used in object instance mask)"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/color [uint] [uint] [uint]",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetObjectAnnotationColor),
		"Set the labeling color of an object [r, g, b]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/mobility",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::GetObjectMobility),
		"Is the object static or movable?"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/show",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetShowObject),
		"Show object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/hide",
		FDispatcherDelegate::CreateRaw(this, &FObjectHandler::SetHideObject),
		"Hide object"
	);
}

AActor* GetActor(const TArray<FString>& Args)
{
	FString ActorId = Args[0];
	AActor* Actor = GetActorById(FUnrealcvServer::Get().GetGameWorld(), ActorId);
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

FExecStatus FObjectHandler::GetObjectLocation(const TArray<FString>& Args)
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
FExecStatus FObjectHandler::SetObjectLocation(const TArray<FString>& Args)
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

FExecStatus FObjectHandler::GetObjectRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FRotator Rotation = Controller.GetRotation();

	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FObjectHandler::SetObjectRotation(const TArray<FString>& Args)
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


FExecStatus FObjectHandler::GetObjectAnnotationColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FColor AnnotationColor;
	Controller.GetAnnotationColor(AnnotationColor);
	return FExecStatus::OK(AnnotationColor.ToString());
}

FExecStatus FObjectHandler::SetObjectAnnotationColor(const TArray<FString>& Args)
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

FExecStatus FObjectHandler::GetObjectMobility(const TArray<FString>& Args)
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

FExecStatus FObjectHandler::SetShowObject(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Show();
	return FExecStatus::OK();
}

FExecStatus FObjectHandler::SetHideObject(const TArray<FString>& Args)
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
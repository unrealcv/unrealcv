// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "ObjectHandler.h"
#include "ObjectPainter.h"
#include "ActorController.h"
#include "VertexSensor.h"
#include "StrFormatter.h"

FExecStatus GetObjectMobility(const TArray<FString>& Args);

AActor* GetActor(const TArray<FString>& Args);


FExecStatus GetActorList(const TArray<FString>& Args);
FExecStatus GetActorLocation(const TArray<FString>& Args);
FExecStatus SetActorLocation(const TArray<FString>& Args);
FExecStatus GetActorRotation(const TArray<FString>& Args);
FExecStatus SetActorRotation(const TArray<FString>& Args);
FExecStatus GetActorVertexLocation(const TArray<FString>& Args);
FExecStatus GetActorAnnotationColor(const TArray<FString>& Args);
FExecStatus SetActorAnnotationColor(const TArray<FString>& Args);
FExecStatus GetActorMobility(const TArray<FString>& Args);
FExecStatus SetShowActor(const TArray<FString>& Args);
FExecStatus SetHideActor(const TArray<FString>& Args);

void FObjectCommandHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /objects",
		FDispatcherDelegate::CreateStatic(GetActorList),
		"Get the name of all objects"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/location",
		FDispatcherDelegate::CreateStatic(GetActorLocation),
		"Get object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/location [float] [float] [float]",
		FDispatcherDelegate::CreateStatic(SetActorLocation),
		"Set object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/rotation",
		FDispatcherDelegate::CreateStatic(GetActorRotation),
		"Get object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateStatic(SetActorRotation),
		"Set object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/vertex_location",
		FDispatcherDelegate::CreateStatic(GetActorVertexLocation),
		"Get vertex location"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/color",
		FDispatcherDelegate::CreateStatic(GetActorAnnotationColor),
		"Get the labeling color of an object (used in object instance mask)"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/color [uint] [uint] [uint]",
		FDispatcherDelegate::CreateStatic(SetActorAnnotationColor),
		"Set the labeling color of an object [r, g, b]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/mobility",
		FDispatcherDelegate::CreateStatic(GetActorMobility),
		"Is the object static or movable?"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/show",
		FDispatcherDelegate::CreateStatic(SetShowActor),
		"Show object"
	);

	CommandDispatcher->BindCommand(
		"vset /object/[str]/hide",
		FDispatcherDelegate::CreateStatic(SetHideActor),
		"Hide object"
	);
}

AActor* GetActor(const TArray<FString>& Args)
{
	FString ActorId = Args[0];
	AActor* Actor = GetActorById(FUE4CVServer::Get().GetGameWorld(), ActorId);
	return Actor;
}

FExecStatus GetActorList(const TArray<FString>& Args)
{
	UWorld* GameWorld = FUE4CVServer::Get().GetGameWorld();
	TArray<AActor*> ActorList = GetActorPtrList(GameWorld);

	FString StrActorList;
	for (AActor* Actor : ActorList)
	{
		StrActorList += FString::Printf(TEXT("%s "), *Actor->GetName());
	}
	return FExecStatus::OK(StrActorList);
}

FExecStatus GetActorLocation(const TArray<FString>& Args)
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
FExecStatus SetActorLocation(const TArray<FString>& Args)
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

FExecStatus GetActorRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	FRotator Rotation = Controller.GetRotation();

	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus SetActorRotation(const TArray<FString>& Args)
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

FExecStatus GetActorVertexLocation(const TArray<FString>& Args)
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

FExecStatus GetActorAnnotationColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");
	FActorController Controller(Actor);

	FColor AnnotationColor;
	bool bSuccess = Controller.GetAnnotationColor(AnnotationColor);
	if (bSuccess)
	{
		return FExecStatus::OK(AnnotationColor.ToString());
	}
	else
	{
		return FExecStatus::Error("The annotation color is empty");
	}
}

FExecStatus SetActorAnnotationColor(const TArray<FString>& Args)
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

FExecStatus GetActorMobility(const TArray<FString>& Args)
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

FExecStatus SetShowActor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Show();
	return FExecStatus::OK();
}

FExecStatus SetHideActor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	if (!Actor) return FExecStatus::Error("Can not find object");

	FActorController Controller(Actor);
	Controller.Hide();
	return FExecStatus::OK();
}

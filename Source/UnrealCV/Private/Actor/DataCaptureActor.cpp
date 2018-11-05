// Weichao Qiu @ 2018
#include "DataCaptureActor.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "EngineUtils.h"
#include "Runtime/Engine/Classes/Components/MaterialBillboardComponent.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformFile.h"
#include "Runtime/Core/Public/HAL/PlatformFilemanager.h"
#include "Runtime/Engine/Classes/Animation/SkeletalMeshActor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/Engine/Public/TimerManager.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "Actor/FusionCameraActor.h"
#include "BPFunctionLib/VisionBPLib.h"
#include "BPFunctionLib/JsonObjectBP.h"
#include "Puppeteer.h"
#include "FusionCamSensor.h"
// #include "VertexSensorComponent.h"

// Sets default values
ADataCaptureActor::ADataCaptureActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// MeshComponent = CreateDefaultSubobject<UMeshComponent>(TEXT("SceneRoot"));
	// RootComponent = MeshComponent; 

	bCaptureImage = true;
	bCaptureSegMask = true;
	bCaptureDepth = true;
	bCaptureNormal = true;

	bCaptureSceneSummary = true;
	bCaptureSceneInfo = true;
	bCaptureAnnotationColor = true;
	bCaptureVertex = false;
	bCapturePuppeteer = true;

	CaptureInterval = 1.0f;
	SimDuration = -1.0f; // Disable simulation duration 
	ImageIdType = EImageId::GameFrameId;
	TimeDilation = 1.0f;
	ImageIdType = EImageId::RecordedFrameId;
	bAddTimestamp = true;
	// Set default to dump folder

	FolderStructure = EFolderStructure::Tree;

	// Create a Material billboard to represent our actor
	Billboard = CreateDefaultSubobject<UMaterialBillboardComponent>(TEXT("BillboardComponent"));
	if (!IsRunningCommandlet() && (Billboard != NULL))
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAsset(TEXT("/Engine/EditorMaterials/HelpActorMaterial"));
		Billboard->AddElement(MaterialAsset.Object, nullptr, false, 32.0f, 32.0f, nullptr);
		// Billboard->SetupAttachment(RootComponent);
		Billboard->bIsEditorOnly = true;
		Billboard->bHiddenInGame = true;
	}
	RootComponent = Billboard;
}

void ADataCaptureActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Note: Can not put this in CTOR, in ctor the GetWorld is nullptr
}

// Note: what is the difference?
void ADataCaptureActor::PostActorCreated()
{
	Super::PostActorCreated();
	DataFolder.Path = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir(), GetWorld()->GetMapName());
}

// Called when the game starts or when spawned
void ADataCaptureActor::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), this->TimeDilation);

	// Bind capture event to functions
	// Start a timer for data capture
	// Shuffle the scene randomly
	FTimerHandle CaptureTimerHandle, SimTimerHandle;

	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(
		CaptureTimerHandle, 
		this, 
		// Note: We want to control the interval with a real world time, not a game time
		&ADataCaptureActor::CaptureFrame, CaptureInterval * TimeDilation, true);

	World->GetTimerManager().SetTimer(
		SimTimerHandle,
		this,
		&ADataCaptureActor::ExitGame, SimDuration, false);


	if (bAddTimestamp)
	{
		// See the format here https://en.wikipedia.org/wiki/ISO_8601
		// or in python
		FString TimestampStr = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M"));
		FinalDataFolder = FPaths::Combine(DataFolder.Path, TimestampStr);
	}
	else
	{
		FinalDataFolder = DataFolder.Path;
	}
}

// Called every frame
void ADataCaptureActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADataCaptureActor::CaptureFrame()
{
	// Define the behavior for capturing a frame
	// CaptureAnnotationColor(); // no need to be per-frame
	CaptureImage();
	CaptureScene();
	CaptureJoint();
	CaptureVertex();
	CapturePuppeteer();
	FrameCounter += 1;
	// CaptureSceneSummary();
	// CaptureSceneInfo();
}

// Utility function to read 3D joint information 
FJsonObjectBP ReadJointInfo(AActor* Actor)
// FJsonObjectBP ReadJointInfo(USkeletalMeshComponent* SkelComponent)
{
	FJsonObjectBP JsonObjectBP;
	
	// ASkeletalMeshActor* SkelMeshActor = Cast<ASkeletalMeshActor>(Actor);
	if (!IsValid(Actor)) return JsonObjectBP;

	UActorComponent* ActorComponent = Actor->GetComponentByClass(USkeletalMeshComponent::StaticClass());
	// USkeletalMeshComponent* SkelComponent = SkelMeshActor->GetSkeletalMeshComponent();
	USkeletalMeshComponent* SkelComponent = Cast<USkeletalMeshComponent>(ActorComponent);

	if (!IsValid(SkelComponent))
	{
		return JsonObjectBP;
	}
	

	TMap<FString, FJsonObjectBP> HumanInfo;

	HumanInfo.Emplace("ActorName", FJsonObjectBP(Actor->GetName()));
	HumanInfo.Emplace("ActorLocation", FJsonObjectBP(Actor->GetActorLocation()));
	HumanInfo.Emplace("ActorRotation", FJsonObjectBP(Actor->GetActorRotation()));

	TArray<FString> IncludedBones;
	TArray<FString> BoneNames;
	TArray<FJsonObjectBP> BoneTransforms;
	UVisionBPLib::GetBoneTransformJson(SkelComponent, IncludedBones, BoneNames, BoneTransforms);
	HumanInfo.Emplace("LocalJoints", FJsonObjectBP(BoneNames, BoneTransforms));
	UVisionBPLib::GetBoneTransformJson(SkelComponent, IncludedBones, BoneNames, BoneTransforms, true);
	HumanInfo.Emplace("WorldJoints", FJsonObjectBP(BoneNames, BoneTransforms));

	return FJsonObjectBP(HumanInfo);
}

// Utility function to read 3D vertex information
FJsonObjectBP ReadVertexInfo(AActor* Actor)
{
	TArray<FVector> VertexArray;
	UVisionBPLib::GetVertexArray(Actor, VertexArray); 

	TArray<FJsonObjectBP> JsonBPArray;
	for (FVector Loc: VertexArray)
	{
		JsonBPArray.Add(FJsonObjectBP(Loc));
	}
	return FJsonObjectBP(JsonBPArray);	

	// Note: new version uses vertex capture component to capture vertex
	// TArray<UActorComponent*> VertexCaptureComponents = Actor->GetComponentsByClass(UVertexCaptureComponent::StaticClass());
	// if (VertexCaptureComponents.Num() == 0) continue;
	// if (VertexCaptureComponents.Num() > 1)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("More than one VertexCaptureComponent for MeshComponent."));
	// }

	// TArray<FVector> ActorVertexs;
	// for (UActorComponent* VertexCaptureComponent : VertexCaptureComponents)
	// {
	// 	TArray<FVector> ComponentVertexs = Cast<UVertexCaptureComponent>(VertexCaptureComponent)->GetVertexArray();
	// 	// Append to ActorVertexs
	// 	ActorVertexs.Append(ComponentVertexs);
	// }
	// TArray<FJsonObjectBP> JsonBPArray;
	// // TODO: Speed up this
	// for (FVector Loc: ActorVertexs) 
	// {
	// 	JsonBPArray.Add(FJsonObjectBP(Loc));
	// }
	// return FJsonObjectBP(JsonBPArray);
}

UClass* SearchCommonClass(UClass *Class)
{
	// Common classes are
	// AActor -> AStaticMeshActor
	//        -> ASkeletalMeshActor
	//        -> AFusionCameraActor
	return nullptr;
}

void ADataCaptureActor::CaptureScene()
{
	if (!bCaptureSceneInfo) return;

	TArray<AActor*> ActorList;
	UVisionBPLib::GetActorList(ActorList);
	TMap<FString, FJsonObjectBP> ActorsDataMap;

	// Per actor information
	for (AActor* Actor : ActorList)
	{
		TMap<FString, FJsonObjectBP> ActorDetails;
		ActorDetails.Emplace("Name", FJsonObjectBP(Actor->GetName()));
		ActorDetails.Emplace("Location", FJsonObjectBP(Actor->GetActorLocation()));
		ActorDetails.Emplace("Rotation", FJsonObjectBP(Actor->GetActorRotation()));
		ActorDetails.Emplace("ClassType", FJsonObjectBP(Actor->GetClass()->GetName()));
		// ActorDetails.Emplace("CommonClassType")
		// The common class type is more useful.

		if (bCaptureAnnotationColor)
		{
			FColor AnnotationColor;
			UVisionBPLib::GetAnnotationColor(Actor, AnnotationColor);
			ActorDetails.Emplace("AnnotationColor", AnnotationColor);
		}

		ActorsDataMap.Emplace(Actor->GetName(), FJsonObjectBP(ActorDetails));
	}


	FJsonObjectBP JsonObjectBP(ActorsDataMap);
	FString StrData = JsonObjectBP.ToString();
	FString Filename = MakeFilename("", "scene", ".json");
	// Joint and vertex data are too large, so better save them in a seperate file
	UVisionBPLib::SaveData(StrData, Filename);
}

void ADataCaptureActor::CaptureVertex()
{
	if (!bCaptureVertex) return;

	// This is huge to serialize to json, consider a more efficient and compact serialization
	TMap<FString, FJsonObjectBP> VertexDataMap;
	for (TActorIterator<ASkeletalMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		VertexDataMap.Emplace(Actor->GetName(), ReadVertexInfo(Actor));
	}

	FString JsonFilename = MakeFilename("", "vertex", ".json");
	FString JsonStr = FJsonObjectBP(VertexDataMap).ToString(); 
	UVisionBPLib::SaveData(JsonStr, JsonFilename);
}

void ADataCaptureActor::CapturePuppeteer()
{
	// Save puppeteer data to meta data file
	if (!bCapturePuppeteer) return;

	for (APuppeteer* Puppeteer : Puppeteers)
	{
		if (IsValid(Puppeteer))
		{
			// Puppeteer can save data by itself, or return a JsonObjectBP
			FJsonObjectBP JsonObjectBP = Puppeteer->GetState(this);
			FString Filename = MakeFilename("", "puppeteer", ".json");
			UVisionBPLib::SaveData(JsonObjectBP.ToString(), Filename);
		}
	}
}

void ADataCaptureActor::CaptureJoint()
{
	if (!bCaptureJoint)
	{
		return;
	}

	TArray<AActor*> HumanActorList;
	// for (TActorIterator<ASkeletalMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		UActorComponent* SkelComponent = Actor->GetComponentByClass(USkeletalMeshComponent::StaticClass());
		if (IsValid(SkelComponent))
		{ 
			HumanActorList.Add(*ActorItr);
		}
	}

	// Capture 3D human keypoints
	// Iterate over human in this scene and save data to disk
	TMap<FString, FJsonObjectBP> JointInfoMap;

	for (AActor* HumanActor : HumanActorList)
	{
		FString HumanName = HumanActor->GetName();
		JointInfoMap.Emplace(HumanName, ReadJointInfo(HumanActor));
	}

	FString JsonFilename = MakeFilename("", "joint", ".json");
	FString JsonStr = FJsonObjectBP(JointInfoMap).ToString(); 
	UVisionBPLib::SaveData(JsonStr, JsonFilename);
}

void ADataCaptureActor::CaptureSceneSummary()
{
	if (!bCaptureSceneSummary) return;
	// A summary of the scene in this frame
}

void ADataCaptureActor::CaptureImageFromSensor(FString SensorName, UFusionCamSensor* Sensor)
{
	// FString CameraName = CameraActor->GetName();
	FString LitFilename = MakeFilename(SensorName, "lit", ".png");
	FString SegFilename = MakeFilename(SensorName, "seg", ".png");
	FString DepthFilename = MakeFilename(SensorName, "depth", ".npy");
	FString JsonFilename = MakeFilename(SensorName, "caminfo", ".json");
	FString NormalFilename = MakeFilename(SensorName, "normal", ".png");

	if (bCaptureImage)
	{
		int Width, Height;
		TArray<FColor> LitData;
		if (bLitSlow)
		{
			Sensor->GetLit(LitData, Width, Height, ELitMode::Slow);
		}
		else
		{
			Sensor->GetLit(LitData, Width, Height, ELitMode::Lit);
		}
		UE_LOG(LogTemp, Display, TEXT("Save lit image to %s"), *LitFilename);
		UVisionBPLib::SavePng(LitData, Width, Height, LitFilename);
	}

	if (bCaptureSegMask)
	{
		int Width, Height;
		TArray<FColor> SegData;
		Sensor->GetSeg(SegData, Width, Height);
		UVisionBPLib::SavePng(SegData, Width, Height, SegFilename);
	}

	if (bCaptureDepth)
	{
		int Width, Height;
		TArray<float> DepthData;
		Sensor->GetDepth(DepthData, Width, Height);
		UVisionBPLib::SaveNpy(DepthData, Width, Height, DepthFilename);
	}

	if (bCaptureNormal)
	{
		int Width, Height;
		TArray<FColor> NormalData;
		Sensor->GetNormal(NormalData, Width, Height);
		UVisionBPLib::SavePng(NormalData, Width, Height, NormalFilename);
	}

	// TArray<float> DepthData;
	// CameraActor->FusionCamSensor->GetDepth(DepthData, Width, Height);
	
	TArray<FString> Keys = {
		"SensorName", 
		"Location", 
		"Rotation", 
		"FilmWidth", 
		"FilmHeight",
		"Fov",
		// "IntrinsicMatrix"
		};
	// Save camera information
	FMatrix Matrix;
	TArray<FJsonObjectBP> Values
	{
		FJsonObjectBP(SensorName),
		FJsonObjectBP(Sensor->GetSensorLocation()),
		FJsonObjectBP(Sensor->GetSensorRotation()),
		FJsonObjectBP(Sensor->GetFilmWidth()),
		FJsonObjectBP(Sensor->GetFilmHeight()),
		FJsonObjectBP(Sensor->GetSensorFOV()),
		// FJsonObjectBP(Matrix)
	};

	// USerializeBPLib::VectorToJson();
	FJsonObjectBP JsonObject = USerializeBPLib::TMapToJson(Keys, Values);
	FString JsonStr = USerializeBPLib::JsonToStr(JsonObject);
	JsonFilename = FPaths::ConvertRelativePathToFull(DataFolder.Path, JsonFilename);
	UVisionBPLib::SaveData(JsonStr, JsonFilename);
}

void ADataCaptureActor::CaptureImage()
{
	// Iterate all cameras in the scene and capture images
	for (ACamSensorActor* CameraActor : Sensors)
	{
		if (!IsValid(CameraActor)) continue;
		
		// Get all sensors in this camera actor
		TArray<FString> SensorNames = CameraActor->GetSensorNames();
		TArray<UFusionCamSensor*> Sensors = CameraActor->GetSensors();

		if (SensorNames.Num() != Sensors.Num())
		{
			UE_LOG(LogTemp, Warning, TEXT("The number of CameraNames and Cameras are mismatch."));
			continue;
		}

		for (int i = 0; i < Sensors.Num(); i++)
		{
			this->CaptureImageFromSensor(SensorNames[i], Sensors[i]);
		}

	}
	FString ScreenMessage = FString::Printf(TEXT("%d frames / %d images are captured from %d cameras"), FrameCounter, FrameCounter * Sensors.Num(), Sensors.Num());
	GEngine->AddOnScreenDebugMessage(-1, -1.0f, FColor::Green, *ScreenMessage);
}

void ADataCaptureActor::ExitGame() { 
	// FGenericPlatformMisc::RequestExit(false);
	// GetWorld()->Exec(GetWorld(), TEXT("quit")); // TODO: Find a better way to implement this
	GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
}


FString ADataCaptureActor::MakeFilename(FString CameraName, FString DataType, FString FileExtension)
{
	int FrameNumber;
	switch (ImageIdType)
	{
		case EImageId::GameFrameId: FrameNumber = UVisionBPLib::FrameNumber(); break;
		case EImageId::RecordedFrameId: FrameNumber = FrameCounter; break;
		default: FrameNumber = UVisionBPLib::FrameNumber();
	}

	FString Filename;
	if (CameraName.IsEmpty())
	{
		if (FolderStructure == EFolderStructure::Tree)
		{
			Filename = FString::Printf(
				TEXT("%s/%08d%s"), 
				*DataType,
				FrameNumber,
				*FileExtension
			);
		}	
		if (FolderStructure == EFolderStructure::Flat)
		{
			Filename = FString::Printf(
				TEXT("%s_%08d%s"), 
				*DataType,
				FrameNumber,
				*FileExtension
			);
		}
	}
	else
	{
		if (FolderStructure == EFolderStructure::Tree)
		{
			Filename = FString::Printf(
				TEXT("%s/%s/%08d%s"), 
				*CameraName, 
				*DataType,
				FrameNumber,
				*FileExtension
			);
		}	
		if (FolderStructure == EFolderStructure::Flat)
		{
			Filename = FString::Printf(
				TEXT("%s_%s_%08d%s"), 
				*CameraName, 
				*DataType,
				FrameNumber,
				*FileExtension
			);
		}
	}


	Filename = FPaths::ConvertRelativePathToFull(FinalDataFolder, Filename);
	return Filename;
}


#if WITH_EDITOR
void ADataCaptureActor::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	// This is important, otherwise the change will be lost
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADataCaptureActor, bListSensors))
	{
		Sensors.Empty();
		// TArray<UObject *> SensorObjects;
		// GetObjectsOfClass(AFusionCameraActor::StaticClass(), SensorObjects, false);
		// for (UObject *Obj : SensorObjects)
		// {
		// 	Sensors.Add(Cast<AFusionCameraActor>(Obj));
		// }
		for (TActorIterator<ACamSensorActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			ACamSensorActor* Actor = *ActorItr;
			Sensors.Add(Actor);
		}
		bListSensors = false;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ADataCaptureActor, bListPuppeteers))
	{
		Puppeteers.Empty();
		for (TActorIterator<APuppeteer> ActorItr(GetWorld()); ActorItr; ++ActorItr)
		{
			APuppeteer* Actor = *ActorItr;
			Puppeteers.Add(Actor);
		}
		bListPuppeteers = false;
	}
}
#endif
#include "UnrealCVPrivate.h"
#include "TestActor.h"
#include "FusionCamSensor.h"

// Run a test command
void ExecCommand(FString Command);

void TestSensorBasic();
void TestSensorList();
void TestCameraBasic();
void TestBPControl();
void TestAnnotator();

ATestActor::ATestActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATestActor::BeginPlay()
{
	Super::BeginPlay();

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TestSensorBasic"),
		TEXT("Run basic sensor test"),
		FConsoleCommandDelegate::CreateStatic(TestSensorBasic)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TestCameraBasic"),
		TEXT("Run basic camera test"),
		FConsoleCommandDelegate::CreateStatic(TestCameraBasic)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TestSensorList"),
		TEXT("Run basic sensor test"),
		FConsoleCommandDelegate::CreateStatic(TestSensorList)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TestBP"),
		TEXT("Test BP control through unrealcv"),
		FConsoleCommandDelegate::CreateStatic(TestBPControl)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("TestAnnotator"),
		TEXT("Test the object instance annotation color"),
		FConsoleCommandDelegate::CreateStatic(TestAnnotator)
	);

	IConsoleObject* Benchmark = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("benchmark"),
		TEXT("Run the command following benchmark"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &ATestActor::SetBenchmarkCmd)
	);
}

void ATestActor::SetBenchmarkCmd(const TArray<FString>& Args)
{
	BenchmarkCmd = "";
	for (FString Arg : Args)
	{
		BenchmarkCmd += FString::Printf(TEXT("%s "), *Arg);
	}
}

void ATestActor::Tick(float DeltaSeconds)
{
	if (BenchmarkCmd.Len() != 0)
	{
		ExecCommand(BenchmarkCmd);
	}

	if (bRunningTest)
	{
		//TArray<FString> Args;
		//RunTest(Args);
		for (FConsoleCommandDelegate Func : TickFunctions)
		{
			Func.Execute();
		}
	}
}

void ExecCommand(FString Command)
{
	// FString Msg = FString::Printf(TEXT("%d : %s"), GFrameNumber, *Command);
	// ScreenLog(Msg);

	//FStringOutputDevice StringOutputDevice(TEXT("StringOutputDevice")); // seems this is not the right way to use it
	//IConsoleManager::Get().ProcessUserConsoleInput(*Command, StringOutputDevice, FUE4CVServer::Get().GetGameWorld());

	FConsoleOutputDevice ConsoleOutputDevice(FUE4CVServer::Get().GetGameWorld()->GetGameViewport()->ViewportConsole);
	IConsoleManager::Get().ProcessUserConsoleInput(*Command, ConsoleOutputDevice, FUE4CVServer::Get().GetGameWorld());

	// How to get the output of the Console command??

	//ScreenLog(StringOutputDevice);
	// ScreenLog(ConsoleOutputDevice);
}

void TestSensorBasic()
{
	//ExecCommand(TEXT("vset /viewmode object_mask"));
	//ExecCommand(TEXT("vset /viewmode lit"));
	//ExecCommand(TEXT("vset /viewmode depth"));
	ExecCommand(TEXT("vget /sensors"));
	ExecCommand(TEXT("vget /sensor/0/info"));
	// Lit
	ExecCommand(TEXT("vget /sensor/0/lit          png"));
	ExecCommand(TEXT("vget /sensor/0/lit          C:/temp/sensor_lit.png"));
	ExecCommand(TEXT("vget /sensor/0/lit          C:/temp/sensor_lit.bmp"));
	// Depth
	ExecCommand(TEXT("vget /sensor/0/depth        npy"));
	ExecCommand(TEXT("vget /sensor/0/depth        C:/temp/sensor_depth.exr"));
	ExecCommand(TEXT("vget /sensor/0/depth        C:/temp/sensor_depth.npy"));
	// Surface normal
	ExecCommand(TEXT("vget /sensor/0/normal       C:/temp/sensor_normal.png"));
	ExecCommand(TEXT("vget /sensor/0/normal       png"));
	// Object mask
	ExecCommand(TEXT("vget /sensor/0/object_mask  C:/temp/sensor_object_mask.png"));
	ExecCommand(TEXT("vget /sensor/0/object_mask  png"));
}

void TestCameraBasic()
{
	// Another version, lit
	ExecCommand(TEXT("vget /camera/0/lit          png"));
	ExecCommand(TEXT("vget /camera/0/lit          C:/temp/camera_lit.png"));
	// Depth
	ExecCommand(TEXT("vget /camera/0/depth        npy"));
	ExecCommand(TEXT("vget /camera/0/depth        C:/temp/camera_depth.npy"));
	// Surface normal
	ExecCommand(TEXT("vget /camera/0/normal       png"));
	ExecCommand(TEXT("vget /camera/0/normal       C:/temp/camera_normal.png"));
	// Object mask
	//ExecCommand(TEXT("vget /camera/0/object_mask  png"));
	//ExecCommand(TEXT("vget /camera/0/object_mask  C:/temp/camera_object_mask.png"));
}

void TestBPControl()
{
	ExecCommand(TEXT(""));
}

void TestAnnotator()
{
	/*
	ExecCommand(TEXT("vget /sensor/0/object_mask png"));

	FObjInstanceAnnotator Annotator;

	FString ObjectId; 

	FColor AnnotationColor;
	Annotator.GetObjInstanceColor(ObjectId, AnnotationColor);
	Annotator.SetObjInstanceColor(ObjectId, AnnotationColor);
	*/
}


void TestSensorList()
{
	// UWorld* World = FUE4CVServer::Get().GetGameWorld();
	// TArray<UObject*> ObjectList;
	// bool bIncludeDerivedClasses = false;
	// EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	// EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	// GetObjectsOfClass(UFusionCamSensor::StaticClass(), ObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);
    //
	// // Test the valid of FusionSensors
	// // For Debug
	// for (TObjectIterator<UFusionCamSensor> Itr; Itr; ++Itr)
	// {
	// 	// Access the subclass instance with the * or -> operators.
	// 	UFusionCamSensor *Component = *Itr;
	// 	UE_LOG(LogUnrealCV, Warning, TEXT("Component: %s"), *Component->GetName());
	// 	UE_LOG(LogUnrealCV, Warning, TEXT("IsTemplate: %s"), Component->IsTemplate() ? TEXT("True"): TEXT("False"));
	// 	if (Component->GetWorld())
	// 	{
	// 		UE_LOG(LogUnrealCV, Warning, TEXT("PIE World: %s"), *WorldTypeStr( Component->GetWorld()->WorldType));
	// 		UE_LOG(LogUnrealCV, Warning, TEXT("World: %s"), *Component->GetWorld()->GetName());
	// 		UE_LOG(LogUnrealCV, Warning, TEXT("Same world?: %s"), Component->GetWorld() == FUE4CVServer::Get().GetGameWorld() ? TEXT("True") : TEXT("False"));
	// 	}
	// 	if (Component->TextureTarget)
	// 	{
	// 		UE_LOG(LogUnrealCV, Warning, TEXT("Component->TextureTarget: %s"), *Component->TextureTarget->GetName());
	// 	}
	// 	ScreenLog(Itr->GetName());
	// }
    //
	// GetObjectsOfClass(UFusionCamSensor::StaticClass(), ObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);
	// for (TObjectIterator<UFusionCamSensor> Itr; Itr; ++Itr)
	// {
	// 	// Access the subclass instance with the * or -> operators.
	// 	UFusionCamSensor *Component = *Itr;
	// 	ScreenLog(Itr->GetName());
	// }
	// return FExecStatus::OK();
	ExecCommand(TEXT("vget /sensors"));
}

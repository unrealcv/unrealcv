// Weichao Qiu @ 2018
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JsonObjectBP.h"
#include "DataCaptureActor.generated.h"


UENUM()
enum class EFolderStructure
{
	Flat, // Put all files in the root folder
	Tree, // Organize by folders
};

UENUM()
enum class EImageId
{
	GameFrameId,
	RecordedFrameId,
};

/** An actor to capture data from the scene.
\image html https://i.imgur.com/MdOxOZ3.png
*/
UCLASS()
class UNREALCV_API ADataCaptureActor : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ADataCaptureActor();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	FDirectoryPath DataFolder;

	// Add Timestamp if necessary
	FString FinalDataFolder;

	/** Do not support resume, merge two generated batch later if needed */
	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	bool bAddTimestamp;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	EFolderStructure FolderStructure;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	bool bListPuppeteers;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	TArray<class APuppeteer*> Puppeteers;

	/** How frequent to capture data to the disk, 0 means capture every frame, negative value means no automatic capture 
	 * Note: this is the real world time, not the game world time which can be dilated.
	*/
	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	float CaptureInterval;

	/** The duration of simulation, stop after this period of time */
	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	float SimDuration;

	/** Reduce the simulation speed to a percentage */
	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	float TimeDilation;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture")
	EImageId ImageIdType;

	/** How many frames have been saved in this session */
	int FrameCounter;

	/** --------- Per camera information ----------- */
	void CaptureFrame();

	/** A button to list all sensors in the map */
	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bListSensors;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	TArray<class ACamSensorActor*> Sensors;


	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bCaptureImage;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bLitSlow;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bCaptureSegMask;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bCaptureDepth;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Camera Setting")
	bool bCaptureNormal;

	void CaptureImage();
	
	void CaptureImageFromSensor(FString SensorName, class UFusionCamSensor* Sensor);

	/** ---------- Scene information ---------- */
	/** Because the scene is static, we only need the first frame for a better speed */

	/** Capture summary of the scene, how many objects, how many cameras, etc. */
	void CaptureSceneSummary(); // Combine information into a single json file

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Scene Setting")
	bool bOnlyFirstFrame;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Scene Setting")
	bool bCaptureSceneSummary;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Scene Setting")
	bool bCaptureSceneInfo;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Scene Setting")
	bool bCaptureAnnotationColor;

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Scene Setting")
	bool bCapturePuppeteer;

	/** Capture the scene details */
	void CaptureScene();

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Joint Setting", meta=(DisplayName="Capture 3D Joint"))
	bool bCaptureJoint;

	void CaptureJoint();

	UPROPERTY(EditInstanceOnly, Category = "DataCapture| Vertex Setting", meta=(DisplayName="Capture 3D Vertex"))
	bool bCaptureVertex;

	void CaptureVertex();

	void CapturePuppeteer();

	void ExitGame();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostActorCreated() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

	FString MakeFilename(FString CameraName, FString DataType, FString FileExtension);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPROPERTY()
	class UMeshComponent* MeshComponent;

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPROPERTY()
	class UMaterialBillboardComponent* Billboard;

	UClass* SearchCommonClass(UClass* Class);
};

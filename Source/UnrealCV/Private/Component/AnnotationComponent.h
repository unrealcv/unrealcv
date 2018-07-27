// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Public/SkeletalRenderPublic.h"

#include "AnnotationComponent.generated.h"

enum class EParentMeshType
{
	None,
	StaticMesh,
	SkelMesh,
};

// TODO: Might need to annotate every frame if there are new actors got spawned
/** A proxy component class to render annotation color
 * Should be attached to a MeshComponent to provide annotation color for AnnotationCamSensor
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class UAnnotationComponent : public UMeshComponent
{
	GENERATED_BODY()

public:
	UAnnotationComponent(const FObjectInitializer& ObjectInitializer);

	// UPROPERTY()
	// UStaticMeshComponent* StaticMeshComponent;

	// UPROPERTY()
	// USkeletalMeshComponent* SkeletalMeshComponent;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	virtual void TickComponent(float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction * ThisTickFunction) override;

	

	FColor AnnotationColor;

private:
	// FParentMeshInfo ParentMeshInfo;
	TSharedPtr<class FParentMeshInfo> ParentMeshInfo;

	UPROPERTY()
	UMaterial* AnnotationMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* AnnotationMID;
};

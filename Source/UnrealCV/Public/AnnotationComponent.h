// Weichao Qiu @ 2017
#pragma once

#include "AnnotationComponent.generated.h"

/** A proxy component class to render annotation color
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class UAnnotationComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:
	UAnnotationComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY()
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY()
	USkeletalMeshComponent* SkeletalMeshComponent;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;

	UPROPERTY()
	FColor AnnotationColor;

private:
	UPROPERTY()
	UMaterial* AnnotationMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* AnnotationMID;
};

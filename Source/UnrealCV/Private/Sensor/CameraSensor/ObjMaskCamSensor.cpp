#include "UnrealCVPrivate.h"
#include "ObjMaskCamSensor.h"

UObjMaskSensor::UObjMaskSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

	this->ShowFlags.SetMaterials(false);
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetBSPTriangles(true);
	this->ShowFlags.SetVertexColors(true);
	this->ShowFlags.SetPostProcessing(false);
	this->ShowFlags.SetHMDDistortion(false);
	this->ShowFlags.SetTonemapper(false); // This won't take effect here

	GVertexColorViewMode = EVertexColorViewMode::Color;
	// this->ShowFlags.SetRendering(false); // This will disable the rendering
}

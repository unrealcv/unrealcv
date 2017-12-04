#include "UnrealCVPrivate.h"
#include "CommandDispatcher.h"
#include "SensorHandler.h"

FExecStatus GetSensorLit(const TArray<FString>& Args);
FExecStatus GetSensorInfo(const TArray<FString>& Args);

void FSensorHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /sensor/[uint]/lit png",
		FDispatcherDelegate::CreateStatic(GetSensorLit),
		"Get png binary data from lit sensor"
	);

	CommandDispatcher->BindCommand(
		"vget /sensor/[uint]/info",
		FDispatcherDelegate::CreateStatic(GetSensorInfo),
		"Get sensor information"
	);

	// CommandDispatcher->BindCommand(
	//     "vget /sensor/[uint]/depth npy",
	//     , //
	//     "Get depth from depth sensor",
	// );
	//
	// CommandDispatcher->BindCommand(
	//     "vget /sensor/[uint]/object_mask npy",
	// );
	//
	// CommandDispatcher->BindCommand(
	//     "vget /sensors",
	//     //
	//     "List all sensors in the scene"
	// );
}


FExecStatus GetSensorLit(const TArray<FString>& Args)
{
	return FExecStatus::OK();
}

FExecStatus GetSensorInfo(const TArray<FString>& Args)
{
	ScreenLog("Hello World");
	return FExecStatus::OK();
}

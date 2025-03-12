#pragma once
#include "CommandHandler.h"
#include "Runtime/InputCore/Classes/InputCoreTypes.h"

class FActionHandler : public FCommandHandler
{
public:
	void RegisterCommands();

private:
	/** vset /action/game/pause */
	FExecStatus PauseGame(const TArray<FString>& Args);

	/** vset /action/game/resume */
	FExecStatus ResumeGame(const TArray<FString>& Args);

	/** vget /action/game/is_paused */
	FExecStatus GetIsPaused(const TArray<FString>& Args); 

	/** vset /action/game/level */
	FExecStatus OpenLevel(const TArray<FString>& Args);

	/** vset /action/input/enable */
	FExecStatus EnableInput(const TArray<FString>& Args);

	/** vset /action/input/disable */
	FExecStatus DisableInput(const TArray<FString>& Args);

	/** vset /action/eyes_distance [float] */
	FExecStatus SetStereoDistance(const TArray<FString>& Args);
	/** vset /action/clean_garbage */
	FExecStatus GarbageCollection(const TArray<FString>& Args);

	FExecStatus SetFixedFPS(const TArray<FString>& Args);

	/** vset /action/keyboard [key_name] [delta] */
	FExecStatus Keyboard(const TArray<FString>& Args);

	TFunction<void(void)> GetReleaseKey(FKey Key);
};

#pragma once
#include "CommandHandler.h"

class FActionCommandHandler : public FCommandHandler
{
public:
	FActionCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	/** vset /action/game/pause */
	FExecStatus PauseGame(const TArray<FString>& Args);

	/** vset /action/game/resume */
	FExecStatus ResumeGame(const TArray<FString>& Args);

	/** vset /action/game/level */
	FExecStatus OpenLevel(const TArray<FString>& Args);

	/** vset /action/input/enable */
	FExecStatus EnableInput(const TArray<FString>& Args);

	/** vset /action/input/disable */
	FExecStatus DisableInput(const TArray<FString>& Args);

	/** vset /action/eyes_distance [float] */
	FExecStatus SetStereoDistance(const TArray<FString>& Args);

	/** vset /action/keyboard [key_name] [delta] */
	FExecStatus Keyboard(const TArray<FString>& Args);
	TFunction<void(void)> GetReleaseKey(FKey Key);
};

/**
ScreenCapture(.h,.cpp) contains the ways to capture screenshot from UE4.
There are several ways to capture screenshot with different limitations
1. Sync capture
	This operation will wait until the capture finished, but the limitation is
	it can only be used within UE4 editor, can not be used in a standalone game.

2. Async capture using the built-in functions of UE4

3. Async capture using a customized GameViewport

4. Async capture using a USceneCaptureComponent2D
*/
#pragma once
#include "ExecStatus.h"
FExecStatus ScreenCaptureAsyncByQuery();
FExecStatus ScreenCaptureAsyncByQuery(const FString& FullFilename);
FExecStatus ScreenCaptureSync(const FString& FullFilename);

/** FIXME: The callback version is not correctly implemented yet. */

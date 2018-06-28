// Weichao Qiu @ 2018
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Misc/DateTime.h"
#include "Runtime/Core/Public/Misc/Timespan.h"

class FBenchmarkMeter
{
public:
	int Counter;
	double AverageTime;
	FDateTime TicTime;
	FString MeterName;

	FBenchmarkMeter(FString InMeterName)
	{
		MeterName = InMeterName;
		AverageTime = 0;
		Counter = 0;
	}

	/** Get FPS */
	int GetFPS()
	{
		return 1000 / AverageTime;
	}

	int GetCounter()
	{
		return Counter;
	}

	/** Get average time */
	float GetAverageTime()
	{
		return AverageTime;
	}

	FString GetName()
	{
		return MeterName;
	}

	void Tic()
	{
		TicTime = FDateTime::Now();
	}

	void Toc()
	{
		FTimespan Delta = FDateTime::Now() - TicTime;
		double TotalMilliseconds = Delta.GetTotalMilliseconds();
		AverageTime = (AverageTime * Counter + TotalMilliseconds) / (Counter + 1);
		Counter += 1;
	}
};
#pragma once

#include "TestActor.generated.h"

UCLASS()
class ATestActor : public AActor
{
    GENERATED_BODY()

public:
	ATestActor(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;

	virtual void Tick(float FloatSeconds) override;

	void StartTest();

	void StopTest();

    void SetBenchmarkCmd(const TArray<FString>& Args);

private:
	bool bRunningTest;
	TArray<FConsoleCommandDelegate> TickFunctions;

    FString BenchmarkCmd;

	void LitBenchmark();
};

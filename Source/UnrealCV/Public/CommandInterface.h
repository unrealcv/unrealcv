#pragma once

#include "Runtime/CoreUObject/Public/UObject/Interface.h"
#include "CommandInterface.generated.h"

UINTERFACE(Blueprintable)
class UCommandInterface : public UInterface
{
    GENERATED_BODY()
};

class ICommandInterface
{    
    GENERATED_BODY()

public:
	UFUNCTION()
	virtual void BindCommands() = 0;

    UFUNCTION()
    virtual void UnbindCommands() = 0;
};

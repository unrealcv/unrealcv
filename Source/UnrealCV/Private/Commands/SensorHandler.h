#pragma once

#include "CommandHandler.h"

class FSensorHandler : public FCommandHandler
{
public:
    FSensorHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
    { }

    void RegisterCommands();


};

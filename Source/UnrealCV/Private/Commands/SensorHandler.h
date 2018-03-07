#pragma once

#include "CommandHandler.h"

/** Handle vget/vset /sensor/... commands */
class FSensorHandler : public FCommandHandler
{
public:
	void RegisterCommands();
};

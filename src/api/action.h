#pragma once

#include "../data/actionenum.h"
#include "../data/paperaccount.h"

#include "client.h"

namespace daytrender
{
	namespace action
	{
		typedef bool (*action_func)(const Client*, double);
		typedef bool (*paper_func)(PaperAccount&, double);

		extern action_func actions[Action::COUNT];
		extern paper_func paper_actions[Action::COUNT];
	}
};
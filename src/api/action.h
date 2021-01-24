#pragma once

#include "../data/actionenum.h"

#include "client.h"
#include "paperaccount.h"

namespace daytrender
{
	namespace action
	{
		typedef bool (*ActionFunc)(Client*, const std::string&, double);
		typedef bool (*PaperFunc)(PaperAccount&, double);

		extern ActionFunc actions[Action::COUNT];
		extern PaperFunc paper_actions[Action::COUNT];
	}
};
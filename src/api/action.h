#pragma once

#include "../data/actionenum.h"
#include "../data/paperaccount.h"

#include "client.h"

namespace daytrender
{
	namespace action
	{
		typedef bool (*ActionFunc)(const Client*, const std::string& ticker, double);
		typedef bool (*PaperFunc)(PaperAccount&, double);

		extern ActionFunc actions[Action::COUNT];
		extern PaperFunc paper_actions[Action::COUNT];
	}
};
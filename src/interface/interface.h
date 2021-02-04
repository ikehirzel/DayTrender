#pragma once

#include <vector>
#include "../api/paperaccount.h"

namespace daytrender
{
	namespace interface
	{
		std::vector<PaperAccount> backtest(int algo_index, int asset_index, bool shorting_enabled,
			int granularity, const std::vector<int>& ranges);
	}
}
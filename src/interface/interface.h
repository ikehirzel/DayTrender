#pragma once

#include <vector>
#include "../api/paperaccount.h"

namespace daytrender
{
	namespace interface
	{
		std::vector<PaperAccount> backtest(int algo_index, int asset_index, double principal,
			bool shorting_enabled, int min_range, int max_range, int granularity,
			const std::vector<int>& test_ranges);
	}
}
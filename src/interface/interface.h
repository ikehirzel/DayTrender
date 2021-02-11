#pragma once

#include <vector>
#include "../api/paperaccount.h"
#include "../api/strategy.h"
#include "../api/client.h"
#include "../api/asset.h"

namespace daytrender
{
	namespace interface
	{
		std::vector<PaperAccount> backtest(int strat_index, int asset_index, double principal,
			bool shorting_enabled, int min_range, int max_range, int granularity,
			const std::vector<int>& test_ranges);
		PaperAccount backtest(const Asset* asset);
		std::vector<PaperAccount> backtest(Client* client, const Strategy* strat, const std::vector<int>& ranges);
	}
}
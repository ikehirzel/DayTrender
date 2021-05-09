#ifndef DAYTRENDER_BACKTEST_H
#define DAYTRENDER_BACKTEST_H

// local includes
#include <api/client.h>
#include <api/strategy.h>
#include <data/asset.h>
#include <data/paperaccount.h>

// standard library
#include <vector>


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

#endif

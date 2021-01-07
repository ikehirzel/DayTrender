#include "asset.h"

#include "candle.h"
#include "action.h"
#include "interval.h"
#include "../api/oandaclient.h"
#include "../api/alpacaclient.h"
#include <hirzel/fountain.h>

namespace daytrender
{
	const char* asset_labels[] = ASSET_LABELS;
	double paper_initials[ASSET_TYPE_COUNT][2] = PAPER_INITIALS;
	unsigned backtest_intervals[ASSET_TYPE_COUNT][3] = BACKTEST_INTERVALS;

	Asset::Asset(int assetIndex, TradeClient *client, const std::string &ticker, TradeAlgorithm* algo,
		int interval, const std::vector<int>& _ranges, bool _paper)
	{
		if (client && algo && !_ranges.empty())
		{
			live = true;
		}
		else
		{
			warningf("Arguments were not sufficient, asset '%s' cannot go live", ticker);
		}

		this->client = client;
		this->algo = algo;
		this->ticker = ticker;
		this->interval = interval;
		candle_count = _ranges[0];
		type = assetIndex;
		data.ranges = _ranges;
		paper = _paper;
		paperAccount = PaperAccount(PAPER_ACCOUNT_INITIAL, paper_initials[type][0], paper_initials[type][1], interval, _ranges);
	}
	
	void Asset::update()
	{
		bool shouldUpdate = tick++ % (interval / 60U) == 0;

		if (shouldUpdate)
		{
			if (!algo)
			{
				warningf("%s: Algorithm has not been initialized! Update cannot continue.", ticker);
				return;
			}

			data.candles.clear();
			data.candles = client->getCandles(ticker, interval, candle_count);

			if (!algo->process(data))
			{
				errorf("Failed to process algorithm for %s", ticker);
			}
			else
			{
				// paper trading
				if (paper)
				{
					paper_actions[data.action](paperAccount, maxRisk);
				}
				// live trading
				else
				{
					actions[data.action](client, maxRisk);
				}
			}

		}
	}
}
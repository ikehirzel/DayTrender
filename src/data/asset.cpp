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
	unsigned int backtest_intervals[ASSET_TYPE_COUNT][3] = BACKTEST_INTERVALS;

	Asset::Asset(unsigned int assetIndex, TradeClient *client, const std::string &ticker, TradeAlgorithm* algo,
		unsigned int interval, unsigned int window)
	{
		if(client && algo)
		{
			live = true;
		}

		this->client = client;
		this->algo = algo;
		this->ticker = ticker;
		this->interval = interval;
		this->window = window;
		type = assetIndex;
		paperAccount = PaperAccount(PAPER_ACCOUNT_INITIAL, paper_initials[type][0], paper_initials[type][1], interval, window);
	}
	
	void Asset::update()
	{
		bool shouldUpdate = tick++ % (interval / 60U) == 0;

		if (shouldUpdate)
		{
			candleset candles = client->getCandles(ticker, interval, window);
			if(!algo)
			{
				warningf("%s: Algorithm has not been initialized!", ticker);
				return;
			}
			algorithm_data algodata = algo->process(candles);
			data.candle_data.candles = candles;
			data.candle_data.interval = interval;
			data.algo_data = algodata;

			// paper trading
			if(paper)
			{
				paper_actions[algodata.action](paperAccount, maxRisk);
			}
			// live trading
			else
			{
				actions[algodata.action](client, maxRisk);
			}
		}
	}
}
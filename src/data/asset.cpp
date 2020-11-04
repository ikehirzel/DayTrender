#include "asset.h"

#include "candle.h"
#include "interval.h"
#include "action.h"
#include "paperaccount.h"
#include "../api/oandaclient.h"
#include "../api/alpacaclient.h"
#include <hirzel/fountain.h>

#include <future>

#define ASSET_NAME	"Asset"

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

		std::vector<std::vector<unsigned int>> intervals = BACKTEST_INTERVALS;
		std::vector<std::vector<double>> initials = PAPER_INITIALS;

		this->client = client;
		this->algo = algo;
		this->ticker = ticker;
		this->interval = interval;
		this->window = window;

		backtestIntervals = intervals[assetIndex];
		
		this->basePaperAccount = PaperAccount(PAPER_ACCOUNT_INITIAL, initials[assetIndex][0],
			initials[assetIndex][1], interval, window);

		paperAccount = PaperAccount(basePaperAccount);

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
			algorithm_data algodata = algo->process(candles, candles.size() - 1, window);
			data.first = candles;
			data.second = algodata;

			// paper trading
			if(paper)
			{
				paper_actions[algodata.second](&paperAccount, maxRisk);
			}
			// live trading
			else
			{
				actions[algodata.second](client, maxRisk);
			}
		}
	}

	PaperAccount Asset::backtest()
	{
		//how to call futures
		//futures[i] = std::async(std::launch::async, &Asset::backtest, this, algorithm, c, inter, testWindow);
		//useful for find best window
		//unsigned int size = (MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW;
		candleset candles = client->getCandles(ticker, interval);
		PaperAccount account(basePaperAccount, interval, window);
		unsigned int index = window - 1;
		// staging the price so that it doesn't tade preemptively
		account.setPrice(candles[index].close);
		//loop for moving window
		for (index = window; index < candles.size(); index++)
		{
			//updating the data we have to work with
			account.setPrice(candles[index].close);
			//calculating move to make
			algorithm_data data = algo->process(candles, index, window);
			//dealing with the action in a paper trading context
			paper_actions[data.second](&account, maxRisk);
		}
		return account;
	}
}
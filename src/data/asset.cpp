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

	Asset::Asset(unsigned int assetIndex, TradeClient *client, TradeAlgorithm* algo,
		const std::string &ticker, unsigned int interval, unsigned int window)
	{
		actions[ACTION_NOTHING] = &Asset::nothing;
		actions[ACTION_SELL] = &Asset::sell;
		actions[ACTION_BUY] = &Asset::buy;

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

	void Asset::buy(PaperAccount* account)
	{
		if (account)
		{
			double shares = (maxRisk * account->getBalance() * (1.0 - account->getFee())) / account->getPrice();
			if (shares < account->getMinimum())
			{
				return;
			}
			account->buy(shares);
		}
		else
		{
			errorf("Live buying is not implemented yet!");
		}
	}

	void Asset::sell(PaperAccount* account)
	{
		if (account)
		{
			double shares = account->getShares();
			if (shares >= account->getMinimum())
			{
				account->sell(shares);
			}
		}
		else
		{
			errorf("Live selling is not implemented yet!");
		}
	}
	
	void Asset::update()
	{
		bool shouldUpdate = tick++ % (interval / 60U) == 0;

		if (shouldUpdate)
		{
			candleset candles = client->getCandles(ticker, interval, window);
			if(!algo)
			{
				warningf("Algorithm is not defined!");
				return;
			}
			algorithm_data algodata = algo->process(candles, candles.size() - 1, window);
			data.first = { interval, candles };
			data.second = algodata;
			(this->*actions[algodata.second])(nullptr);
		}
	}

	PaperAccount Asset::backtest(TradeAlgorithm* algo, const candleset &candles,
		unsigned int inter, unsigned int win)
	{
		PaperAccount account(basePaperAccount, inter, win);
		//pre staging the price so that it doesn't tade preemptively
		unsigned int index = window - 1;
		account.setPrice(candles[index][3]);

		//loop for moving window
		for (index = window; index < candles.size(); index++)
		{
			//updating the data we have to work with
			account.setPrice(candles[index][3]);
			//std::cout << "\t\t\tPRICE: " << account.getPrice() << std::endl;

			//calculating move to make
			algorithm_data data = algo->process(candles, index, window);
			//dealing with the action in a paper trading context
			//handleAction(data.second, &account);
			(this->*actions[data.second])(&account);
		}

		return account;
	}
	
	PaperAccount Asset::testCurrentConstraints()
	{
		return backtest(algo, client->getCandles(ticker, interval), interval, window);
	}
	
	PaperAccount Asset::findBestWindow(TradeAlgorithm* algorithm, unsigned int inter, const candleset& c)
	{
		PaperAccount best(basePaperAccount);
		
		unsigned int size = (MAX_ALGORITHM_WINDOW + 1) - MIN_ALGORITHM_WINDOW;
		
		std::vector<std::future<PaperAccount>> futures(size);
		std::vector<PaperAccount> accounts(size);
		
		for(unsigned int i = 0; i < size; i++)
		{
			unsigned int testWindow = i + MIN_ALGORITHM_WINDOW;
			futures[i] = std::async(std::launch::async, &Asset::backtest,
				this, algorithm, c, inter, testWindow);
		}
		
		for (unsigned int i = 0; i < size; i++)
		{
			accounts[i] = futures[i].get();
			if (accounts[i].equity() > best.equity())
			{
				best = accounts[i];
			}
		}
		
		return best;
	}
	
	PaperAccount Asset::findBestWindow(TradeAlgorithm* algorithm, unsigned int inter)
	{
		candleset c = client->getCandles(ticker, inter);
		return findBestWindow(algorithm, inter, c);
	}
	
	PaperAccount Asset::findBestConstraints(TradeAlgorithm* algorithm)
	{	
		PaperAccount best;
		
		for(unsigned int inter : backtestIntervals)
		{
			candleset c = client->getCandles(ticker, inter);
			PaperAccount acc = findBestWindow(algorithm, inter, c);
			if(acc.netYearReturn() > best.netYearReturn())
			{
				best = acc;
			}
		}
		return best;
	}
}
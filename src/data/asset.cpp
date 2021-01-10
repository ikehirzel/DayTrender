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

	Asset::Asset(int _assetIndex, TradeClient *_client, const std::string &_ticker, TradeAlgorithm* _algo,
		int _interval, double _risk, const std::vector<int>& _ranges, bool _paper)
	{
		if (_client && _algo && !_ranges.empty())
		{
			live = true;
		}
		else
		{
			warningf("Arguments were not sufficient, asset '%s' cannot go live", ticker);
		}

		client = _client;
		algo = _algo;
		ticker = _ticker;
		interval = _interval;
		if (_ranges.size() > 1)
		{
			for (int i = 1; i < _ranges.size(); i++)
			{
				if (_ranges[i] > candle_count) candle_count = _ranges[i];
			}
		}
		candle_count += _ranges[0];
		type = _assetIndex;
		data.ranges = _ranges;
		paper = _paper;
		risk = _risk;
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
					paper_actions[data.action](paperAccount, risk);
				}
				// live trading
				else
				{
					actions[data.action](client, risk);
				}
			}

		}
	}

	asset_info Asset::getAssetInfo() const
	{
		asset_info out;
		if (live)
		{
			out.live = live;
			out.paper = paper;
			out.risk = risk;
			if (paper)
			{
				out.shares = paperAccount.getShares();
			}
			else
			{
				errorf("Get asset info not yet implemented");
				//out = client->get
				out.shares = 0.0;
			}
		}
		return out;
	}
}
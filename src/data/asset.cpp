#include "asset.h"

#include "candle.h"
#include "interval.h"

#include "../api/action.h"
#include "../api/client.h"
#include <hirzel/fountain.h>

namespace daytrender
{
	Asset::Asset(int _type, const Client* _client, const std::string &_ticker, const Algorithm* _algo,
		int _interval, double _risk, const std::vector<int>& _ranges, bool _paper)
	{
		if (!_ranges.empty())
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
		type = _type;
		ranges = _ranges;
		paper = _paper;
		risk = _risk;

		paperAccount = PaperAccount(PAPER_ACCOUNT_INITIAL, client->paper_fee(), client->paper_minimum(), interval, _ranges);
	}
	
	void Asset::update()
	{
		bool shouldUpdate = tick++ % (interval / 60U) == 0;

		if (shouldUpdate)
		{
			// clearing old data from algo data
			data.clear();
			data.candles.clear();

			// making new algo data
			candleset candles = client->get_candles(ticker, interval, candle_count);
			data = algo->process(candles, ranges);
		
			// error handling
			if (data.err)
			{
				errorf("Action could not be handled %s", ticker);
				return;
			}
		
			// paper trading
			if (paper)
			{
				paperAccount.setPrice(candles.back().close);
				action::paper_actions[data.action](paperAccount, risk);
			}
			// live trading
			else
			{
				action::actions[data.action](client, risk);
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
#include "asset.h"

#include "candle.h"
#include "../api/action.h"
#include "../api/client.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

namespace daytrender
{
	Asset::Asset(int type, const Client* client, const std::string &ticker, const Algorithm* algo,
		int interval, double risk, const std::vector<int>& ranges, bool paper)
	{
		if (!ranges.empty())
		{
			_live = true;
		}
		else
		{
			warningf("Arguments were not sufficient, asset '%s' cannot go live", ticker);
		}

		_client = client;
		_algo = algo;
		_ticker = ticker;
		_interval = interval;

		if (ranges.size() > 1)
		{
			for (int i = 1; i < ranges.size(); i++)
			{
				if (ranges[i] > _candle_count) _candle_count = ranges[i];
			}
		}

		_candle_count += ranges[0];
		_type = type;
		_ranges = ranges;
		_paper = paper;
		_risk = risk;

		_paper_account = PaperAccount(PAPER_ACCOUNT_INITIAL, client->paper_fee(), client->paper_minimum(), interval, _ranges);
	}
	
	void Asset::update()
	{
		long long curr_time = hirzel::sys::get_seconds();
		_last_update = curr_time - (curr_time % _interval);

		infof("Updating %s...", _ticker);

		// making new algo data
		CandleSet candles = _client->get_candles(_ticker, _interval, _candle_count);
		_data = _algo->process(candles, _ranges);
	
		// error handling
		if (_data.error())
		{
			errorf("Action could not be handled %s", _ticker);
			return;
		}
	
		// paper trading
		if (_paper)
		{
			_paper_account.setPrice(candles.back().close);
			action::paper_actions[_data.action()](_paper_account, _risk);
		}
		// live trading
		else
		{
			action::actions[_data.action()](_client, _ticker, _risk);
			std::cout << "live action!\n";
		}
		std::cout << "Finished updatin!\n";
		infof("Finished updating!");
	}

	AssetInfo Asset::info() const
	{
		AssetInfo out;
		if (_live)
		{
			out.live = _live;
			out.paper = _paper;
			out.risk = _risk;
			if (_paper)
			{
				out.shares = _paper_account.getShares();
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

	bool Asset::should_update() const
	{
		if ((hirzel::sys::get_seconds() - _last_update) > _interval)
			{
				if (_client->market_open())
				{
					return true;
				}
				else
				{
					errorf("%s: The market is closed! Updating anyway...", _ticker);
					return true;
				}
			}
			return false;
	}
}
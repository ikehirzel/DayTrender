#include "asset.h"

#include "candle.h"
#include "../api/action.h"
#include "../api/client.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

namespace daytrender
{
	Asset::Asset(int type, Client* client, const std::string &ticker, const Algorithm* algo,
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
		_client->increment_assets();
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
		if (risk > 1.0)
		{
			warningf("%s: risk (%f) should be a maximum of 1.0. It will be readjusted.", ticker, risk);
			risk = 1.0;
		}
		else if (risk < 0.0)
		{
			warningf("%s: risk (%f) should me a minimum of 0.0; It will be readjusted.", ticker, risk);
			risk = 0.0;
		}

		_risk = risk;
		double fee = client->paper_fee();
		double minimum = client->paper_minimum();
		double principal = client->get_price(ticker);
		_paper_account = PaperAccount(PAPER_ACCOUNT_INITIAL, fee, minimum, principal, interval, _ranges);
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
			_paper_account.update_price(candles.back().close);
			action::paper_actions[_data.action()](_paper_account, _risk);
		}
		// live trading
		else
		{
			action::actions[_data.action()](_client, _ticker, _risk);
			std::cout << "live action!\n";
		}
	}

	bool Asset::should_update() const
	{
		if ((hirzel::sys::get_seconds() - _last_update) > _interval)
			{
				if (_client->market_open())
				{
					return true;
				}
			}
			return false;
	}
}
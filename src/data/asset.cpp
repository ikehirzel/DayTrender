#include "asset.h"

#include "candle.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>
#include <cmath>

namespace daytrender
{
	Asset::Asset(int type, Client* client, const std::string &ticker, const Algorithm* algo,
		int interval, double risk, const std::vector<int>& ranges, bool paper)
	{
		// assigning variables
		_client = client;
		_client->increment_assets();
		_algo = algo;
		_ticker = ticker;
		_interval = interval;
		_type = type;
		//_paper = paper;
		_risk = risk;
		_ranges = ranges;

		// calculating candle_count
		for (int i = 0; i < ranges.size(); i++)
		{
			if (ranges[i] > _candle_count) _candle_count = ranges[i];
		}
		_candle_count += _algo->data_length();

		// readjusting risk
		if (_risk > 1.0)
		{
			warningf("%s: risk (%f) should be a maximum of 1.0. It will be readjusted.", _ticker, _risk);
			_risk = 1.0;
		}
		else if (_risk < 0.0)
		{
			warningf("%s: risk (%f) should me a minimum of 0.0; It will be readjusted.", _ticker, _risk);
			_risk = 0.0;
		}
		
		if (!_ranges.empty())
		{
			_live = true;
			successf("Successfully initialized asset: '%s'", _ticker);
		}
		else
		{
			errorf("Arguments were not sufficient, asset '%s' cannot go live", _ticker);
		}
	}
	
	void Asset::update()
	{
		long long curr_time = hirzel::sys::get_seconds();
		_last_update = curr_time - (curr_time % _interval);

		infof("Updating %s...", _ticker);

		// getting candlestick data from client
		CandleSet candles = get_candles();
		// 
		if (candles.error())
		{
			errorf("%s: CandleSet error: %s", _ticker, candles.error());
			return;
		}

		// processing the candlestick data gotten from client
		_data = _algo->process(candles, _ranges);

		// error handling
		if (_data.error())
		{
			errorf("%s: Algorithm: %s", _ticker, _data.error());
			return;
		}

		if (_data.candles().error())
		{
			errorf("%s: Algorithm candles: %s", _ticker, _data.candles().error());
			return;
		}
		
		return;

		bool res;
		switch (_data.action())
		{
		case ENTER_LONG:
			res = enter_long();
			successf("%s: Entered long position", _ticker);
			break;

		case EXIT_LONG:
			res = exit_long();
			successf("%s: Exited long position", _ticker);
			break;

		case ENTER_SHORT:
			res = enter_short();
			successf("%s: Entered short position", _ticker);
			break;

		case EXIT_SHORT:
			res = exit_short();
			successf("%s: Exited short position", _ticker);
			break;

		default:
			res = true;
			successf("%s: No action taken");
			break;
		}

		if (!res)
		{
			errorf("%s: Failed to handle action", _ticker);
		}
	}

	bool Asset::should_update() const
	{
		if (_live)
		{
			if ((hirzel::sys::get_seconds() - _last_update) > _interval)
			{
				int secs = _client->secs_till_market_close();
				if (secs > 0)
				{
					
					if (secs <= _closeout_buffer * 60)
					return true;
				}
			}
		}
		else
		{

		}
		return false;
	}

	bool Asset::enter_long()
	{
		successf("%s: entering long position", _ticker);

		// api calls
		AccountInfo acct = _client->get_account_info();
		double fee = _client->fee();
		double order_minimum = _client->order_minimum();
		double curr_shares = get_shares();
		double curr_price = get_price();

		// getting share of current buying power
		double base_buying_power = acct.balance() * acct.leverage() * _client->risk() / _client->asset_count();
		// getting amount not spent
		double available_bp = base_buying_power - curr_shares * curr_price;
		// calculating amount of shares that can be purchaes
		double max_shares = available_bp / (1.0 + fee);
		double shares_to_order = std::floor(max_shares / order_minimum) * order_minimum;
		// exit if not ordering any
		if (shares_to_order == 0.0) return true;
		// calling in order
		return false; // but not acutally though
		return _client->market_order(_ticker, shares_to_order);
	}

	bool Asset::exit_long()
	{
		double curr_shares = _client->get_shares(_ticker);
		if (curr_shares == 0.0) return true;
		return false;
		return _client->market_order(_ticker, -curr_shares);
	}

	bool Asset::enter_short()
	{
		AccountInfo acct = _client->get_account_info();

		if (_shorting_enabled && acct.shorting_enabled())
		{
			return false;
		}
		return true;
	}

	bool Asset::exit_short()
	{
		AccountInfo acct = _client->get_account_info();
		if (acct.shorting_enabled())
		{
			double curr_shares = _client->get_shares(_ticker);
			if (curr_shares == 0.0) return true;
			if (curr_shares < 0.0) return _client->market_order(_ticker, -curr_shares);
		}
		return true;
	}
}
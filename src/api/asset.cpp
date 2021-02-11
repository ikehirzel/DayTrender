#include "asset.h"

#include "paperaccount.h"
#include "../data/mathutil.h"
#include "../interface/interface.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>

namespace daytrender
{
	Asset::Asset(Client* client, const Strategy* strategy, const std::string& ticker, int type,
		int interval, const std::vector<int>& ranges) :
	_client(client),
	_strategy(strategy),
	_type(type),
	_interval(interval),
	_ticker(ticker),
	_ranges(ranges),
	_live(true)
	{
		for (int i = 0; i < _ranges.size(); i++)
		{
			if (_ranges[i] > _candle_count) _candle_count = _ranges[i];
		}
		_candle_count += _strategy->data_length();

		_client->increment_assets();

		PaperAccount acc = interface::backtest(this);
		double kelly = acc.kelly_criterion();
		_risk = (kelly >= 0.0 ? kelly : 0.0);
	}
	
	void Asset::update()
	{
		// if the client or asset is currently not live, do not update
		if (!_client->is_live() || !_live) return;

		// if the proper amount of time has not passed, do not update
		if ((hirzel::sys::get_seconds() - _last_update) < _interval) return;
		// updating previously updated time
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
		_data = _strategy->execute(candles, _ranges);

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
		
		bool res = false;
		switch (_data.action())
		{
		case ENTER_LONG:
			res = _client->enter_long(_ticker, _risk);
			break;

		case EXIT_LONG:
			res = _client->exit_long(_ticker);
			successf("%s: Exited long position", _ticker);
			break;

		case ENTER_SHORT:
			res = _client->enter_short(_ticker, _risk);
			successf("%s: Entered short position", _ticker);
			break;

		case EXIT_SHORT:
			res = _client->exit_short(_ticker);
			successf("%s: Exited short position", _ticker);
			break;

		case NOTHING:
			res = true;
			successf("%s: No action taken", _ticker);
			break;

		default:
			errorf("%s: Invalid action received from strategy: %d", _ticker, _data.action());
			_live = false;
			break;
		}

		if (!res)
		{
			_live = false;
			errorf("%s: Failed to handle action. Asset is no longer live.", _ticker);
		}
	}
}
#include "asset.h"

#include "../data/mathutil.h"

#include <hirzel/sysutil.h>
#include <hirzel/fountain.h>
#include <cmath>

namespace daytrender
{
	Asset::Asset(Client* client, const Algorithm* algo, const std::string& ticker, int type,
		int interval, const std::vector<int>& ranges) :
	_client(client),
	_algo(algo),
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
		_candle_count += _algo->data_length();

		_client->increment_assets();
	}
	
	void Asset::update()
	{
		// if the client is currently not live, do not update
		if (!_client->is_live()) return;

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
}
#include <data/asset.h>

// local inlcudes
#include <data/paperaccount.h>
#include <data/mathutil.h>
#include <interface/backtest.h>

// external libararies
#include <hirzel/util/sys.h>
#include <hirzel/logger.h>
#include <hirzel/data.h>

using namespace hirzel;

namespace daytrender
{
	Asset::Asset(const Data& config, const std::string& dir)
	{
		if (!config["ticker"].is_string() || !config["strategy"].is_string()) return;
		if (!(config["interval"].to_double() > 0.0)) return;

		_ticker = config["ticker"].to_string();
		_interval = config["interval"].to_int();
		const Data& ranges_json = config["ranges"];

		_ranges.resize(ranges_json.size());

		for (int i = 0; i < _ranges.size(); i++)
		{
			_ranges[i] = ranges_json[i].to_int();
			if (_ranges[i] > _candle_count) _candle_count = _ranges[i];
		}
		_candle_count += _strategy.data_length();
		/*
		PaperAccount acc = interface::backtest(this);
		double kelly = acc.kelly_criterion();
		_risk = (kelly >= 0.0 ? kelly : 0.0);
		*/
	}
	
	unsigned Asset::update(Client& client)
	{
		// if the proper amount of time has not passed, do not update
		if ((hirzel::sys::epoch_seconds() - _last_update) < _interval) return NOTHING;

		// updating previously updated time
		long long curr_time = hirzel::sys::epoch_seconds();
		_last_update = curr_time - (curr_time % _interval);

		INFO("Updating %s...", _ticker);

		// getting candlestick data from client
		Result<CandleSet> res = get_candles(client);
		if (!res.ok()) 
		{
			ERROR("Candles: %s", res.error());
			return ERROR;
		}

		CandleSet candles = res.get();

		// processing the candlestick data gotten from client

		///
		///
		/// REPLACE THE MAGIC NUMBER BELOW
		///
		_data = _strategy.execute(candles, _ranges, 15);

		// error handling
		if (_data.error())
		{
			ERROR("%s: Algorithm: %s", _ticker, _data.error());
			return ERROR;
		}
		
		return _data.action();
	}
}
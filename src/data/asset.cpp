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
	Asset::Asset(const Data& config, const std::string& dir) :
	_ticker(config["ticker"].to_string()),
	_interval(config["inteval"].to_uint()),
	_strategy(config["strategy"].to_string(), dir)
	{
		if (!_strategy.is_bound()) return;
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
		Result<PriceHistory> price_res = get_candles(client);
		if (!price_res.ok()) 
		{
			ERROR("Candles: %s", price_res.error());
			return ERROR;
		}

		PriceHistory candles = price_res.get();

		// processing the candlestick data gotten from client

		///
		///
		/// REPLACE THE MAGIC NUMBER BELOW
		///
		Result<Chart> chart_res = _strategy.execute(candles, _ranges);

		if (!chart_res.ok())
		{
			ERROR("%s (%s): %s", _strategy.filename(), _ticker, chart_res.error());
			return ERROR;
		}

		_data = chart_res.get();
		
		return _data.action();
	}
}
#include <data/asset.h>

// local inlcudes
#include <data/paperaccount.h>
#include <data/mathutil.h>
#include <interface/backtest.h>

// external libararies
#include <hirzel/logger.h>

using namespace hirzel;

namespace daytrender
{
	Asset::Asset(const Data& config, const std::string& dir)
	{
		// getting ticker
		if (!config["ticker"].is_string()) throw "ticker is not a string";
		_ticker = config["ticker"].to_string();

		// getting interval
		if (!config["interval"].is_num()) throw "interval is not a number";
		_interval = config["interval"].to_uint();

		// getting strategy
		if (!config["strategy"].is_string()) throw "strategy is not a string";
		try
		{
			_strategy = Strategy(config["strategy"].to_string(), dir);
			if (!_strategy.is_bound()) return;
		}
		catch (std::string err)
		{
			
		}
		

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
	
	unsigned Asset::update(const PriceHistory& hist)
	{
		DEBUG("updating $%s", _ticker);
		// processing the candlestick data gotten from client
		try
		{
			Chart data = _strategy.execute(hist, _ranges);
			return data.action();
		}
		catch (std::string err)
		{
			ERROR("(%s) %s: %s", _ticker, _strategy.filename(), err);
			return ERROR;
		}
	}
}
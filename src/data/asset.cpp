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
		if (!config["ticker"].is_string())
		{
			ERROR("ticker is not a string");
			return;
		}

		_ticker = config["ticker"].to_string();

		if (!config["interval"].is_num())
		{
			ERROR("%s: interval is not a number", _ticker);
			return;
		}

		_interval = config["interval"].to_uint();

		if (!config["strategy"].is_string())
		{
			ERROR("%s: strategy is not a string", _ticker);
			return;
		}

		_strategy = Strategy(config["strategy"].to_string(), dir);
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
	
	unsigned Asset::update(const PriceHistory& hist)
	{
		// processing the candlestick data gotten from client
		Result<Chart> res = _strategy.execute(hist, _ranges);

		if (!res)
		{
			ERROR("(%s) %s: %s", _ticker, _strategy.filename(), res.error());
			return ERROR;
		}

		_data = res.get();
		
		return _data.action();
	}
}
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
	Asset::Asset(const Data& config, const std::string& dir) :
		_interval(get_interval(config)),
		_ranges(get_ranges(config)),
		_candle_count(get_candle_count()),
		_ticker(get_ticker(config)),
		_strategy(get_strategy(config, dir)),
		_risk(get_risk()) { }
	
	unsigned Asset::get_interval(const Data& config) const
	{
		if (!config.contains("interval"))
			throw std::invalid_argument("Asset: interval must be defined in config");

		const Data& interval = config["interval"];

		if (!interval.is_uint())
			throw std::invalid_argument("Asset: interval ("
				+ interval.to_string()
				+ ") must be a natural number");
		
		return interval.to_uint();
	}

	std::vector<unsigned> Asset::get_ranges(const hirzel::Data& config) const
	{
		if (!config.contains("ranges"))
			throw std::invalid_argument("Asset: ranges must be defined in config");

		const Data& ranges = config["ranges"];

		if (!ranges.is_array())
			throw std::invalid_argument("Asset: ranges ("
				+ ranges.to_string()
				+ ") must be an array");

		std::vector<unsigned> out;

		out.reserve(config.size());

		for (const Data& d : config.to_array())
		{
			if (!d.is_num())
				throw std::invalid_argument("Asset: range element ("
					+ d.to_string()
					+ ") must be a number");

			out.push_back((unsigned)d.to_uint());
		}

		return out;
	}

	Strategy Asset::get_strategy(const Data& config, const std::string& dir) const
	{
		if (!config.contains("strategy"))
			throw std::invalid_argument("Asset: strategy must be defined in config");

		const Data& strategy = config["strategy"];

		if (!strategy.is_string())
			throw std::invalid_argument("Asset: strategy ("
				+ strategy.to_string()
				+ ") must be a string");

		return Strategy(strategy.to_string(), dir);
	}

	std::string Asset::get_ticker(const Data& config) const
	{
		if (!config.contains("ticker"))
			throw std::invalid_argument("Asset: ticker must be defined in config");
		
		const Data& ticker = config["ticker"];

		if (!ticker.is_string())
			throw std::invalid_argument("Asset: ticker ("
				+ ticker.to_string()
				+ ") must be a string");

		return ticker.to_string();
	}

	unsigned Asset::get_candle_count() const
	{
		unsigned max = 0;

		for (unsigned u : _ranges)
		{
			if (u > max)
				max = u;
		}

		return max;
	}

	double Asset::get_risk() const
	{
		// PaperAccount acc = interface::backtest(this);
		// double kelly = acc.kelly_criterion();
		// _risk = (kelly >= 0.0 ? kelly : 0.0);
		WARNING("get_risk() is not yet implemented: setting risk to 0.0");
		return 0.0;
	}

	unsigned Asset::update(const PriceHistory& hist)
	{
		DEBUG("updating $%s", _ticker);
		
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
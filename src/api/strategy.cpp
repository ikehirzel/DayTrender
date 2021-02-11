#include "strategy.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#define API_VERSION_CHECK
#include "strategydefs.h"

namespace daytrender
{
	Strategy::Strategy(const std::string& _filepath)
	{
		_filename = hirzel::str::get_filename(_filepath);
		_plugin = new hirzel::Plugin(_filepath, { "indicator_count", "data_length", "strategy", "api_version" });
		
		if (!_plugin->all_bound())
		{
			errorf(_plugin->get_error());
			return;
		}

		int api_version = _plugin->execute<int>("api_version");
		if (api_version != STRATEGY_API_VERSION)
		{
			errorf("%s: api version (%d) did not match current api version: %d)", _filename, api_version, STRATEGY_API_VERSION);
			return;
		}
		_indicator_count = _plugin->execute<int>("indicator_count");
		_data_length = _plugin->execute<int>("data_length");
		_execute = (void(*)(StrategyData&))_plugin->get_func("strategy");
	}

	Strategy::~Strategy()
	{
		delete _plugin;
	}
	
	StrategyData Strategy::execute(const CandleSet& candles, const std::vector<int>& ranges) const
	{

		StrategyData data(ranges, candles);

		if (!_execute)
		{
			errorf("%s: strategy function is not bound!", _filename);
			return data;
		}

		_execute(data);

		if (data.error())
		{
			std::string args_glob;
			const std::vector<int>& ranges = data.ranges();
			for (int i = 0; i < ranges.size(); i++)
			{
				if (i > 0) args_glob += ", ";
				args_glob += std::to_string(ranges[i]);
			}
			errorf("%s(%s): %s", data.label(), args_glob, data.error());
		}

		return data;
	}
}

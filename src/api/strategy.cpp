#include "strategy.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

#include <unordered_map>

#define API_VERSION_CHECK
#include "strategydefs.h"

namespace daytrender
{
	std::unordered_map<std::string, hirzel::Plugin*> Strategy::plugins;

	void Strategy::free_plugins()
	{
		for (const std::pair<std::string, hirzel::Plugin*>& pair : plugins)
		{
			delete pair.second;
		}
	}

	Strategy::Strategy(const std::string& filepath)
	{
		_filename = hirzel::str::get_filename(filepath);

		// fetching pointer corresponding to name
		_plugin = plugins[filepath];

		// if no such pointer exists
		if (!_plugin)
		{
			// initialize new plugin
			_plugin = new hirzel::Plugin(filepath, { "indicator_count", "data_length", "strategy", "api_version" });
			// store pointer in map
			plugins[filepath] = _plugin;
		}
		
		if (_plugin->error())
		{
			ERROR("%s: error: %s", _filename, _plugin->error());
			return;
		}

		int api_version = _plugin->execute<int>("api_version");
		if (api_version != STRATEGY_API_VERSION)
		{
			ERROR("%s: api version (%d) did not match current api version: %d)", _filename, api_version, STRATEGY_API_VERSION);
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
			ERROR("%s: strategy function is not bound!", _filename);
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
			ERROR("%s(%s): %s", data.label(), args_glob, data.error());
		}

		return data;
	}
}

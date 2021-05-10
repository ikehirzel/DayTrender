#include <api/strategy.h>

// local includes
#include <api/versions.h>

// standard library
#include <unordered_map>

// external libararies
#include <hirzel/plugin.h>
#include <hirzel/logger.h>
#include <hirzel/util/str.h>

namespace daytrender
{
	std::unordered_map<std::string, hirzel::Plugin*> Strategy::_plugins;

	void Strategy::free_plugins()
	{
		for (auto p : _plugins)
		{
			delete p.second;
		}
	}

	Strategy::Strategy(const std::string& filepath)
	{
		_filename = hirzel::str::get_filename(filepath);

		// fetching pointer corresponding to name
		_plugin = _plugins[filepath];

		// if no such pointer exists
		if (!_plugin)
		{
			// initialize new plugin
			_plugin = new hirzel::Plugin(filepath, 
			{
				"indicator_count",
				"data_length",
				"execute",
				"api_version"
			});
			
			if (!_plugin->bound())
			{
				ERROR("plugin %s failed to bind", filepath);
				delete _plugin;
				return;
			}
			// store pointer in map
			_plugins[filepath] = _plugin;
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
		_indicator_count = _plugin->execute<uint32_t>("indicator_count");
		_data_length = _plugin->execute<uint32_t>("data_length");
		_execute = (const char *(*)(Chart*))_plugin->get_func("strategy");
		if (!_plugin->bound()) return;
		_bound = true;
	}

	Result<Chart> Strategy::execute(const PriceHistory& candles,
		const std::vector<int>& ranges) const
	{
		if (!_execute) return "execute function is not bound";

		Chart data(ranges, candles, _data_length);

		const char *error = _execute(&data);

		if (error) return error;

		return data;
	}
}

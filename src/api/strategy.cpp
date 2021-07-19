#include <api/strategy.h>

// local includes
#include <api/versions.h>

// standard library
#include <unordered_map>
#include <iostream>

// external libararies
#include <hirzel/plugin.h>
#include <hirzel/logger.h>
#include <hirzel/util/str.h>

#define STRATEGY_DIR "/strategies/"

using hirzel::Plugin;

namespace daytrender
{
	std::unordered_map<std::string, std::shared_ptr<Plugin>> Strategy::_plugins;

	Strategy::Strategy(const std::string& filename, const std::string& dir) :
	_filename(filename)
	{
		// fetching pointer corresponding to name
		_plugin = _plugins[filename];

		// if no such pointer exists
		if (!_plugin)
		{
			_plugin = std::make_shared<Plugin>();
			
			
			if (!_plugin->bind(dir + STRATEGY_DIR + filename))
			{
				throw _plugin->error();
			};
			
			if (!_plugin->bind_functions({
				"indicator_count",
				"data_length",
				"execute",
				"api_version"
			}))
			{
				throw _plugin->error();
			}

			_plugins[filename] = _plugin;
		}

		int api_version = _plugin->execute<int>("api_version");
		if (api_version != STRATEGY_API_VERSION)
		{
			ERROR("%s: api version (%d) did not match current api version: %d)", _filename, api_version, STRATEGY_API_VERSION);
			return;
		}

		_indicator_count = _plugin->execute<uint32_t>("indicator_count");
		_data_length = _plugin->execute<uint32_t>("data_length");
		_execute = (decltype(_execute))_plugin->get_function("execute");
	}


	void segfault_handler(int signal)
	{
		std::cerr << "\033[31merror:\033[0m Segmentation fault caused by strategy.\n";
		std::abort();
	}


	Chart Strategy::execute(const PriceHistory& candles,
		const std::vector<unsigned>& ranges) const
	{
		if (!_execute) throw _filename + ": execute function is not bound";
		// create chart data
		Chart data(ranges, candles, _data_length);

		// execute the strategy
		const char *error = _execute(&data);

		if (error) throw _filename + ": " + std::string(error);

		return data;
	}
}

#include <api/strategy.h>

// local includes
#include <api/versions.h>

// standard library
#include <unordered_map>

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
			_plugin = std::make_shared<Plugin>((std::string)(dir + STRATEGY_DIR + filename),
			(std::vector<std::string>)
			{
				"indicator_count",
				"data_length",
				"execute",
				"api_version"
			});
			
			if (!_plugin->bound() || _plugin->error())
			{
				ERROR("%s: %s", _filename, _plugin->error());
				_plugin.reset();
				return;
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
		_execute = (decltype(_execute))_plugin->get_func("execute");
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

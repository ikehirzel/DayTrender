#include "client.h"

#include "../data/mathutil.h"

#include <hirzel/util/str.h>
#include <hirzel/util/sys.h>
#include <hirzel/plugin.h>
#include <hirzel/logger.h>

#include "api_versions.h"

using namespace hirzel;

namespace daytrender
{
	std::unordered_map<std::string, hirzel::Plugin*> Client::_plugins;

	const char *client_api_func_names[] =
	{
		"init",
		"api_version",
		"get_candles",
		"get_account_info",
		"get_asset_info",
		"market_order",
		"close_all_positions",
		"secs_till_market_close",
		"set_leverage",
		"to_interval",
		"key_count",
		"max_candles",
		"get_error"
	};

	// gets plugin by filepath and caches it if it had to load a new one
	Plugin *Client::get_plugin(const std::string& filepath)
	{
		Plugin *plugin = _plugins[filepath];
		// if the plugin is not already cached, attempt to cache it
		if (!plugin)
		{
			plugin = new hirzel::Plugin(filepath);

			// if plugin has errors, log, delete and exit
			if (plugin->error())
			{
				ERROR("%s: error: %s", filepath, plugin->error());
				delete plugin;
				return nullptr;
			}


			// attempt to bind all api functions by name
			size_t func_count = sizeof(client_api_func_names) / sizeof(char*);
			for (size_t i = 0; i < func_count; i++)
			{
				// bind function
				Function func = plugin->bind_function(client_api_func_names[i]);
				// check if failed to bind
				if (!func)
				{
					ERROR("%s: client function '%s' failed to bind",
						filepath, client_api_func_names[i]);
					delete plugin;
					return nullptr;
				}
			}

			// cache plugin
			_plugins[filepath] = plugin;
		}

		return plugin;
	}

	void Client::free_plugins()
	{
		for (auto p : _plugins)
		{
			delete p.second;
		}
	}

	Client::Client(const std::string& filepath) :
	_filepath(filepath)
	{
		// get plugin
		_plugin = get_plugin(filepath);
	}

	#define FUNC_CHECK() { if (!_plugin) return "client is not bound"; }


	Result<CandleSet> Client::get_candles(const std::string& ticker,
		unsigned interval, unsigned count) const
	{
		FUNC_CHECK();

		if (count == 0)
		{
			count = max_candles();
		}
		else if (count > max_candles())
		{
			return "requested more candles than maximum";
		}

		// CandleSet candles(max, end, interval);
		// bool res = _get_candles(candles, ticker);
		// if (!res) flag_error();
		return CandleSet();
	}

	Result<AccountInfo> Client::get_account_info() const
	{
		FUNC_CHECK();

		if (!client_ok()) return AccountInfo();

		AccountInfo info;
		bool res = _get_account_info(info);
		if (!res) flag_error();

		return info;
	}

	/**
	 * Places an immediately returning order on the market. If the amount
	 * is set to zero, it'll return true and not place an order. If the amount
	 * is positive, it'll place a long order and a short order if the shares
	 * are negative.
	 * 
	 * @param	ticker	the symbol that the client should place the order for
	 * @param	amount	the amount of shares the client should order
	 * @return			a bool representing success or failure of the function
	 */
	bool Client::market_order(const std::string& ticker, double amount)
	{
		FUNC_CHECK();
		if (!client_ok()) return false;
		if (amount == 0.0) return true;
		bool res = _market_order(ticker, amount);
		if (!res) flag_error();
		return res;
	}

	Result<AssetInfo> Client::get_asset_info(const std::string& ticker) const
	{
		FUNC_CHECK();

		AssetInfo info;
		bool res = _get_asset_info(info, ticker);
		if (!res) flag_error();
		return info;
	}

	bool Client::close_all_positions()
	{
		// this function can happen when not live

		bool res = _close_all_positions();
		if (!res) flag_error();

		return res;
	}

	unsigned Client::secs_till_market_close() const
	{

		int seconds;
		bool res = _secs_till_market_close(seconds);
		if (!res) flag_error();

		return seconds;
	}

	std::string Client::to_interval(int interval) const
	{
		if (!client_ok()) return "";
		
		const char* interval_str = _to_interval(interval);
		if (!interval_str)
		{
			return "";
		}

		return std::string(interval_str);
	}
}
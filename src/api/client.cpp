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
		"get_candles"
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
			
			// Binding all of the plugin functions
			#define FUNC_PAIR(x) { (void(**)())&_##x, #x }
			// preparing vector so functions can be iterated over
			std::vector<std::pair<void(**)(), const char*>> funcs =
			{
				FUNC_PAIR(init),
				FUNC_PAIR(api_version),
				FUNC_PAIR(get_candles),
				FUNC_PAIR(get_account_info),
				FUNC_PAIR(get_asset_info),
				FUNC_PAIR(market_order),
				FUNC_PAIR(close_all_positions),
				FUNC_PAIR(secs_till_market_close),
				FUNC_PAIR(set_leverage),
				FUNC_PAIR(to_interval),
				FUNC_PAIR(key_count),
				FUNC_PAIR(max_candles),
				FUNC_PAIR(backtest_intervals),
				FUNC_PAIR(get_error)
			};

			// checking if every function bound correctly
			for (const std::pair<void(**)(), const char*>& f: funcs)
			{
				*f.first = plugin->bind_function(f.second);

				// if failed to bind, log it and exit
				if (!*f.first)
				{
					ERROR("%s: function '%s' failed to bind", filepath, f.second);
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
	_filename(str::get_filename(filepath))
	{
		// get plugin
		_plugin = get_plugin(filepath);
		if (!_plugin) return;	

		_bound = _plugin->bound();
	}

	bool Client::client_ok() const
	{

		if (!_bound)
		{
			ERROR("%s: Client is not bound. Function cannot be called.", _filename);
		}

		return _bound;
	}

	void Client::flag_error() const
	{
		ERROR("%s: %s", _filename, get_error());
		_bound = false;
	}

	CandleSet Client::get_candles(const std::string& ticker, int interval, unsigned max, unsigned end) const
	{
		if (!client_ok()) return CandleSet();

		if (max == 0)
		{
			max = max_candles();
		}
		else if (max > max_candles())
		{
			ERROR("Client '%s': requested more candles than maximum!", _filename);
			return {};
		}

		if (end == 0)
		{
			end = max;
		}
		else if (end > max)
		{
			ERROR("Client '%s': attempted to set candleset end as greater than max", _filename);
			return {};
		}

		CandleSet candles(max, end, interval);
		bool res = _get_candles(candles, ticker);
		if (!res) flag_error();
		return candles;
	}

	AccountInfo Client::get_account_info() const
	{
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
		if (!client_ok()) return false;
		if (amount == 0.0) return true;
		bool res = _market_order(ticker, amount);
		if (!res) flag_error();
		return res;
	}

	AssetInfo Client::get_asset_info(const std::string& ticker) const
	{
		if (!client_ok()) return {};
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

	int Client::secs_till_market_close() const
	{
		if (!client_ok()) return false;

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
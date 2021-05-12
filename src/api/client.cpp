#include <api/client.h>

// local includes
#include <api/versions.h>
#include <data/mathutil.h>

// standard library
#include <cstring>

// external libraries
#include <hirzel/util/str.h>
#include <hirzel/util/sys.h>
#include <hirzel/plugin.h>
#include <hirzel/logger.h>


#define CLIENT_DIR "/clients/"

using namespace hirzel;

namespace daytrender
{
	std::unordered_map<std::string, hirzel::Plugin*> Client::_plugins;

	const char *client_api_func_names[] =
	{
		"init",
		"api_version",
		"get_price_history",
		"get_account",
		"get_position",
		"market_order",
		"close_all_positions",
		"secs_till_market_close",
		"set_leverage",
		"to_interval",
		"key_count",
		"max_candles"
	};

	// gets plugin by filepath and caches it if it had to load a new one
	Plugin *Client::get_plugin(const std::string &filename, const std::string& dir)
	{
		std::string filepath = dir + CLIENT_DIR + filename;
		Plugin *plugin = _plugins[filepath];
		// if the plugin is not already cached, attempt to cache it
		if (!plugin)
		{
			plugin = new hirzel::Plugin(filepath);

			// if plugin has errors, log, delete and exit
			if (plugin->error())
			{
				ERROR(plugin->error());
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
						filename, client_api_func_names[i]);
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

	Client::Client(const std::string& filename, const std::string& dir) :
	_filename(filename)
	{
		// get plugin
		_plugin = get_plugin(filename, dir);

		_init = (decltype(_init))_plugin->get_func("init");
		_market_order = (decltype(_market_order))_plugin->get_func("market_order");
		_close_all_positions = (decltype(_close_all_positions))_plugin->get_func("close_all_positions");
		_set_leverage = (decltype(_set_leverage))_plugin->get_func("set_leverage");
		_get_account = (decltype(_get_account))_plugin->get_func("get_account");
		_get_price_history = (decltype(_get_price_history))_plugin->get_func("get_price_history");
		_get_position = (decltype(_get_position))_plugin->get_func("get_position");
		_to_interval = (decltype(_to_interval))_plugin->get_func("to_interval");
		_secs_till_market_close = (decltype(_secs_till_market_close))_plugin->get_func("secs_till_market_close");

		_api_version = (decltype(_api_version))_plugin->get_func("api_version");
		_key_count = (decltype(_key_count))_plugin->get_func("key_count");
		_max_candles = (decltype(_max_candles))_plugin->get_func("max_candles");
	}

	#define FUNC_CHECK() { if (!_plugin) return "client is not bound"; }


	const char *Client::init(const hirzel::Data& keys)
	{
		unsigned keyc = key_count();
		if (keyc != keys.size()) return nullptr;

		char **key_arr = new char*[keyc];

		for (unsigned i = 0; i < keys.size(); i++)
		{
			const std::string& str = keys[i].to_string();
			key_arr[i] = new char[str.size() + 1];
			strcpy(key_arr[i], str.c_str());
		}

		const char *error = _init((const char**)key_arr);

		for (unsigned i = 0; i< keyc; ++i) delete[] key_arr[i];
		delete[] key_arr;

		return error;
	}

	const char *Client::set_leverage(unsigned leverage)
	{
		return "this function is not implemented yet";
	}


	Result<PriceHistory> Client::get_price_history(const std::string& ticker,
		unsigned interval, unsigned count) const
	{
		cli_func_check();

		if (count == 0)
		{
			count = max_candles();
		}
		else if (count > max_candles())
		{
			return "requested more candles than maximum";
		}
		PriceHistory hist(count, interval);
		const char *error = _get_price_history(&hist, ticker.c_str());
		if (error) return error;
		return hist;
	}

	Result<Account> Client::get_account() const
	{
		cli_func_check();
		Account account;
		const char *error = _get_account(&account);
		if (error) return error;
		return account;
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
	const char *Client::market_order(const std::string& ticker, double amount)
	{
		cli_func_check();
		if (amount == 0.0) return nullptr;
		return _market_order(ticker.c_str(), amount);
	}

	Result<Position> Client::get_position(const std::string& ticker) const
	{
		cli_func_check();

		Position position;
		const char *error = _get_position(&position, ticker.c_str());
		if (error) return error;
		return position;
	}

	const char *Client::close_all_positions()
	{
		// this function can happen when not live
		cli_func_check();
		return _close_all_positions();
	}
}
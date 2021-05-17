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
	std::unordered_map<std::string, std::shared_ptr<Plugin>> Client::_plugins;

	Client::Client(const std::string& filename, const std::string& dir) :
	_filename(filename)
	{
		// get plugin
		_plugin = _plugins[filename];
		// if the plugin is not already cached, attempt to cache it
		if (!_plugin)
		{
			_plugin = std::make_shared<Plugin>((std::string)dir + CLIENT_DIR + filename,
			(std::vector<std::string>)
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
			});

			// if plugin has errors, log, delete and exit
			if (!_plugin->bound() || _plugin->error())
			{
				ERROR(_plugin->error());
				_plugin.reset();
				return;
			}

			// cache plugin
			_plugins[filename] = _plugin;
		}

		// point functions
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
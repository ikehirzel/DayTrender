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
			std::string plugin_dir = dir + CLIENT_DIR + filename;
			DEBUG(plugin_dir);
			_plugin = std::make_shared<Plugin>();
			const char *error = _plugin->bind(plugin_dir);
			if (error)
			{
				ERROR(error);
				_plugin.reset();
				return;
			}
			error = _plugin->bind_functions({
				"init",
				"api_version",
				"get_price_history",
				"get_account",
				"get_position",
				"market_order",
				"secs_till_market_close",
				"set_leverage",
				"to_interval",
				"key_count",
				"max_candles"
			});
			if (error) 
			{
				ERROR(error);
				_plugin.reset();
				return;
			}
			// cache plugin
			_plugins[filename] = _plugin;
		}

		// point functions
		_init = (decltype(_init))_plugin->get_function("init");
		_market_order = (decltype(_market_order))_plugin->get_function("market_order");
		_set_leverage = (decltype(_set_leverage))_plugin->get_function("set_leverage");
		_get_account = (decltype(_get_account))_plugin->get_function("get_account");
		_get_price_history = (decltype(_get_price_history))_plugin->get_function("get_price_history");
		_get_position = (decltype(_get_position))_plugin->get_function("get_position");
		_to_interval = (decltype(_to_interval))_plugin->get_function("to_interval");
		_secs_till_market_close = (decltype(_secs_till_market_close))_plugin->get_function("secs_till_market_close");

		_api_version = (decltype(_api_version))_plugin->get_function("api_version");
		_key_count = (decltype(_key_count))_plugin->get_function("key_count");
		_max_candles = (decltype(_max_candles))_plugin->get_function("max_candles");
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
		cli_func_check();
		return _set_leverage(leverage);
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

	const char *Client::close_position(const Asset& asset)
	{
		Result<Position> res = get_position(asset.ticker());
		if (!res) return res.error();
		Position pos = res.get();
		return market_order(asset.ticker(), -pos.shares());
	}

	const char *Client::close_all_positions(const std::vector<Asset>& assets)
	{
		bool failed = false;
		for (const Asset& a : assets)
		{
			const char *error = close_position(a);
			if (error)
			{
				failed = true;
				ERROR("Failed to close position for $%s: %s", a.ticker(), error);
			}
		}
		return (failed) ? "failed to close all positions" : nullptr;
	}

	const char *Client::enter_position(const Asset& asset, double pct, bool short_shares)
	{
		// if not buying anything, exit
		if (asset.risk() == 0.0) return nullptr;

		// will be -1.0 if short_shares is true or 1.0 if it's false
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// getting current account information
		Result<Account> acc_res = get_account();
		if (!acc_res) return acc_res.error();
		Account acc = acc_res.get();

		// get position information
		Result<Position> pos_res = get_position(asset.ticker());
		if (!pos_res) return pos_res.error();
		Position pos = pos_res.get();


		// pct should equal _risk / risk_sum()

		// base buying power
		double buying_power = (acc.buying_power() + acc.margin_used()) * asset.risk() * pct;

		// if we are already in a position of the same type as requested
		if (pos.shares() * multiplier > 0.0)
		{
			// remove the current share of the buying power
			buying_power -= pos.amt_invested();
		}
		// we are in a position that is opposite to type requested
		else if (pos.shares() * multiplier < 0.0)
		{
			// calculate returns upon exiting position for correct buying power calculation
			buying_power += pos.shares() * pos.price() * (1.0 - multiplier * pos.fee());
		}

		double shares = multiplier * std::floor(((buying_power / (1.0 + pos.fee())) / pos.price()) / pos.minimum()) * pos.minimum();
		DEBUG("Placing order for %f shares!!!", shares);
		
		return market_order(asset.ticker(), shares);
	}

	
	const char *Client::exit_position(const Asset& asset, bool short_shares)
	{
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// get position information
		Result<Position> pos_res = get_position(asset.ticker());
		if (!pos_res) return pos_res.error();
		Position pos = pos_res.get();

		// if we are in an opposite position or have no shares
		if (pos.shares() * multiplier <= 0.0) return nullptr;

		// exit position
		return market_order(asset.ticker(), -pos.shares());
	}
}
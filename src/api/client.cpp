#include "client.h"

#include "../data/mathutil.h"

#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define API_VERSION_CHECK
#include "clientdefs.h"

namespace daytrender
{
	#define FUNC_PAIR(x) { (void(**)())&_##x, #x }

	Client::Client(const json& config, const std::string& directory):
	_closeout_buffer(config["closeout_buffer"].get<int>() * 60),
	_filename(config["filename"].get<std::string>()),
	_history_length(config["history_length"].get<double>()),
	_label(config["label"].get<std::string>()),
	_max_loss(config["max_loss"].get<double>()),
	_risk(config["risk"].get<double>()),
	_shorting_enabled(config["shorting_enabled"].get<bool>())
	{
		int leverage = config["leverage"].get<int>();
		const json& credentials = config["keys"];
		std::vector<std::string> keys(credentials.begin(), credentials.end());

		_plugin = new hirzel::Plugin(directory + _filename);

		// checking if the plugin failed to bind
		if (_plugin->error())
		{
			ERROR("%s: error: %s", _filename, _plugin->error());
			return;
		}

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
			*f.first = _plugin->bind_function(f.second);

			// if it didn't, create an error
			if (!*f.first)
			{
				flag_error();
				return;
			}
		}

		// verifying api version of client matches current one
		int api_version = _api_version();
		if (api_version != CLIENT_API_VERSION)
		{
			ERROR("%s: api version for (%d) did not match current api version: %d)", api_version, CLIENT_API_VERSION);
			return;
		}

		// verifying that the correct amount of credentials passed
		int key_count = _key_count();
		if (key_count != keys.size())
		{
			ERROR("%s: expected %d keys but %d were supplied.", _filename, key_count, keys.size());
			return;
		}
		
		// initializing the client
		if (!_init(credentials))
		{
			flag_error();
			return;
		}

		// setting client leverage
		if (!_set_leverage(leverage))
		{
			flag_error();
			return;
		}

		_bound = true;
		_live = true;
	}

	Client::~Client()
	{
		delete _plugin;
	}

	bool Client::client_ok() const
	{
		if (!_live)
		{
			ERROR("%s: Client is not live. Function cannot be called.", _filename);
			return false;
		}

		if (!_bound)
		{
			ERROR("%s: Client is not bound. Function cannot be called.", _filename);
			return false;
		}

		return true;
	}

	void Client::flag_error() const
	{
		ERROR("%s: %s", _filename, get_error());
		_bound = false;
	}

	void Client::update()
	{
		if (!_bound) return;

		int till_close = secs_till_market_close();

		if (_live)
		{
			// updating pl of client
			AccountInfo info = get_account_info();
			long long curr_time = hirzel::sys::get_seconds();
			_equity_history.push_back({ curr_time, info.equity() });

			while (curr_time - _equity_history[0].first > (long long)(_history_length * 3600))
			{
				_equity_history.erase(_equity_history.begin());
			}

			double prev_equity = _equity_history.front().second;
			_pl = info.equity() - prev_equity;

			// account has lost too much in last interval
			if (_pl <= prev_equity * -_max_loss)
			{
				ERROR("%s client '%s' has undergone %f loss in the last %f hours! Closing all position...", _label, _filename, _pl, _history_length);
				close_all_positions();
				_bound = false;
				ERROR("%s client '%s' has gone offline!", _label, _filename);
				return;
			}

			// checking to see if in range of closeout buffer
			
			if (till_close <= _closeout_buffer)
			{
				INFO("Client '%s': Market will close in %d minutes. Closing all positions...", _filename, _closeout_buffer / 60);
				_live = false;
				if (!close_all_positions())
				{
					ERROR("Client '%s': Failed to close positions!", _filename);
				}
			}
		}
		else
		{
			if (till_close > 0)
			{
				_live = true;
			}
		}
	}

	/**
	 * @brief	Orders max amount of shares orderable
	 * 
	 * @param	ticker			the symbol that will be evalutated
	 * @param	pct				the percentage of available buying power to use
	 * @param	short_shares	true if going short shares, false if going long on shares
	 * 
	 * @return	the amount of shares to order
	 */

	bool Client::enter_position(const std::string& ticker, double asset_risk, bool short_shares)
	{
		// if not buying anything, exit
		if (asset_risk == 0.0) return true;

		// will be -1.0 if short_shares is true or 1.0 if it's false
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// getting current account information
		AccountInfo account = get_account_info();
		// getting position info
		AssetInfo asset = get_asset_info(ticker);

		// base buying power
		double buying_power = (account.buying_power() + account.margin_used()) * _risk
			* (asset_risk / _risk_sum);

		// if we are already in a position of the same type as requested
		if (asset.shares() * multiplier > 0.0)
		{
			// remove the current share of the buying power
			buying_power -= asset.amt_invested();
		}
		// we are in a position that is opposite to type requested
		else if (asset.shares() * multiplier < 0.0)
		{
			// calculate returns upon exiting position for correct buying power calculation
			buying_power += asset.shares() * asset.price() * (1.0 - multiplier * asset.fee());
		}

		double shares = multiplier * std::floor(((buying_power / (1.0 + asset.fee())) / asset.price()) / asset.minimum()) * asset.minimum();
		INFO("Placing order for %f shares!!!", shares);
		return market_order(ticker, shares);
	}

	bool Client::exit_position(const std::string& ticker, bool short_shares)
	{
		double multiplier = (double)short_shares * -2.0 + 1.0;
		AssetInfo info = get_asset_info(ticker);
		// if we are in a short position or have no shares, do nothing
		INFO("%s: Exiting position of %f shares", ticker, info.shares());
		if (info.shares() * multiplier <= 0.0) return true;
		// exit position
		return market_order(ticker, -info.shares());
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
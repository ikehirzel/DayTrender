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
	Client::Client(const std::string& label, const std::string& filepath,
		const std::vector<std::string>& credentials, bool shorting_enabled, double risk,
		double max_loss, int leverage, double history_length, int closeout_buffer)
	{
		_label = label;
		_shorting_enabled = shorting_enabled;
		_risk = risk;
		_filename = hirzel::str::get_filename(filepath);
		_max_loss = max_loss;
		_history_length = history_length;
		_closeout_buffer = closeout_buffer * 60;
		_handle = new hirzel::Plugin(filepath);

		if (!_handle->is_lib_bound())
		{
			errorf("%s failed to bind: %s", _filename, _handle->get_error());
			return;
		}

		int api_version = ((int(*)())_handle->bind_function("api_version"))();
		if (api_version != CLIENT_API_VERSION)
		{
			errorf("%s: api version for (%d) did not match current api version: %d)", api_version, CLIENT_API_VERSION);
			errorf("Failed to initialize client: '%s'", _filename);
			return;
		}

		_init = (bool(*)(const std::vector<std::string>&))_handle->bind_function("init");
		if (!_init) flag_error();

		_get_candles = (bool(*)(CandleSet&, const std::string&))_handle->bind_function("get_candles");
		if (!_get_candles) flag_error();

		_get_account_info = (bool(*)(AccountInfo&))_handle->bind_function("get_account_info");
		if (!_get_account_info) flag_error();

		_get_asset_info = (bool(*)(AssetInfo&, const std::string&))_handle->bind_function("get_asset_info");
		if (!_get_asset_info) flag_error();

		_market_order = (bool(*)(const std::string&, double))_handle->bind_function("market_order");
		if (!_market_order) flag_error();

		_close_all_positions = (bool(*)())_handle->bind_function("close_all_positions");
		if (!_close_all_positions) flag_error();

		_secs_till_market_close = (bool(*)(int&))_handle->bind_function("secs_till_market_close");
		if (!_secs_till_market_close) flag_error();

		_set_leverage = (bool(*)(int))_handle->bind_function("set_leverage");
		if (!_set_leverage) flag_error();

		// getters

		_to_interval = (const char*(*)(int))_handle->bind_function("to_interval");
		if (!_to_interval) flag_error();

		// all of these are guaranteed as a result of compilation and don't need to be checked
		_key_count = (int(*)())_handle->bind_function("key_count");
		_max_candles = (int(*)())_handle->bind_function("max_candles");
		_backtest_intervals = (void(*)(std::vector<int>&))_handle->bind_function("backtest_intervals");
		_get_error = (void(*)(std::string&))_handle->bind_function("get_error");

		_bound = _handle->all_bound();

		if (_bound)
		{

			if (!_init(credentials))
			{
				flag_error();
			}
			else
			{
				_live = true;
			}
			
			if (!set_leverage(leverage))
			{
				_live = false;
			}
		}
		else
		{
			errorf("Failed to bind to all functions of: '%s': %s", _filename, _handle->get_error());
		}
	}

	Client::~Client()
	{
		delete _handle;
	}

	bool Client::func_ok(const char* label, void(*func)()) const
	{
		if (!_live)
		{
			errorf("Client '%s': client is not live.", _filename);
			return false;
		}

		if (!_bound)
		{
			errorf("Client '%s': client is not bound.", _filename);

			return false;
		}

		if (func == nullptr)
		{
			errorf("Client '%s': function is not bound and cannot be executed!", _filename);
			return false;
		}

		return true;
	}

	void Client::flag_error() const
	{
		errorf("%s: %s", _filename, get_error());
		_live = false;
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
				errorf("%s client '%s' has undergone %f loss in the last %f hours! Closing all position...", _label, _filename, _pl, _history_length);
				close_all_positions();
				_bound = false;
				errorf("%s client '%s' has gone offline!", _label, _filename);
				return;
			}

			// checking to see if in range of closeout buffer
			
			if (till_close <= _closeout_buffer)
			{
				infof("Client '%s': Market will close in %d minutes. Closing all positions...", _filename, _closeout_buffer / 60);
				_live = false;
				if (!close_all_positions())
				{
					errorf("Client '%s': Failed to close positions!", _filename);
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
	 * Calculates the amount of shares orderable with current capital and risk allowance.
	 * Returns the api success of the order
	 * 
	 * @param	ticker			the symbol that will be evalutated
	 * @param	pct				the percentage of available buying power to use
	 * @param	short_shares	true if going short shares, false if going long on shares
	 * @return					the amount of shares to order
	 */

	bool Client::enter_position(const std::string& ticker, double pct, bool short_shares)
	{
		// will be -1.0 if short_shares is true or 1.0 if it's false
		double multiplier = (double)short_shares * -2.0 + 1.0;

		// getting current account information
		AccountInfo account = get_account_info();
		// getting position info
		AssetInfo asset = get_asset_info(ticker);

		// base buying power
		double buying_power = (account.buying_power() + account.margin_used()) * _risk / (double)_asset_count;

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

		// limits buying power to a ratio of itself.
		// this is applied afterwards as it logarithmically limits the amount of money to spend
		// when multiple entrances occur before an exit
		buying_power *= pct;

		double shares = multiplier * std::floor(((buying_power / (1.0 + asset.fee())) / asset.price()) / asset.minimum()) * asset.minimum();
		infof("Placing order for %f shares!!!", shares);
		return market_order(ticker, shares);
	}

	bool Client::exit_position(const std::string& ticker, bool short_shares)
	{
		double multiplier = (double)short_shares * -2.0 + 1.0;
		AssetInfo info = get_asset_info(ticker);
		// if we are in a short position or have no shares, do nothing
		infof("%s: Exiting position of %f shares", ticker, info.shares());
		if (info.shares() * multiplier <= 0.0) return true;
		// exit position
		return market_order(ticker, -info.shares());
	}

	CandleSet Client::get_candles(const std::string& ticker, int interval, unsigned max, unsigned end) const
	{
		if (!func_ok("get_candles", (void(*)())_get_candles)) return CandleSet();

		if (max == 0)
		{
			max = max_candles();
		}
		else if (max > max_candles())
		{
			errorf("Client '%s': requested more candles than maximum!", _filename);
			return {};
		}

		if (end == 0)
		{
			end = max;
		}
		else if (end > max)
		{
			errorf("Client '%s': attempted to set candleset end as greater than max", _filename);
			return {};
		}

		CandleSet candles(max, end, interval);
		bool res = _get_candles(candles, ticker);
		if (!res) flag_error();
		return candles;
	}

	AccountInfo Client::get_account_info() const
	{
		if (!func_ok("get_account_info", (void(*)())_get_account_info)) return AccountInfo();

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
		if (!func_ok("market_order", (void(*)())_market_order)) return false;
		if (amount == 0.0) return true;
		bool res = _market_order(ticker, amount);
		if (!res) flag_error();
		return res;
	}

	AssetInfo Client::get_asset_info(const std::string& ticker) const
	{
		if (!func_ok("get_asset_info", (void(*)())_get_asset_info)) return {};
		AssetInfo info;
		bool res = _get_asset_info(info, ticker);
		if (!res) flag_error();
		return info;
	}

	bool Client::close_all_positions()
	{
		// this function can happen when not live

		if (!_close_all_positions)
		{
			errorf("%s: close_all_positions is not bound and cannot be executed!", _filename);
			return false;
		}

		bool res = _close_all_positions();
		if (!res) flag_error();

		return res;
	}

	int Client::secs_till_market_close() const
	{
		if (!func_ok("secs_till_market_close", (void(*)())_secs_till_market_close)) return false;

		int seconds;
		bool res = _secs_till_market_close(seconds);
		if (!res) flag_error();

		return seconds;
	}

	bool Client::set_leverage(int multiplier)
	{
		if (!func_ok("set_leverage", (void(*)())_set_leverage)) return false;

		bool res = _set_leverage(multiplier);
		if (!res) flag_error();

		return res;
	}

	std::string Client::to_interval(int interval) const
	{
		if (!func_ok("to_interval", (void(*)())_to_interval)) return "";
		
		const char* interval_str = _to_interval(interval);
		if (!interval_str)
		{
			return "";
		}

		return std::string(interval_str);
	}
}
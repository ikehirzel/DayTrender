#include "client.h"

#include "../data/mathutil.h"

#include <hirzel/strutil.h>
#include <hirzel/sysutil.h>
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define API_VERSION_CHECK
#include "../data/clientdefs.h"

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

	bool Client::enter_long(const std::string ticker, double pct)
	{
		// api calls
		AccountInfo acct = get_account_info();  
		AssetInfo info = get_asset_info(ticker);

		// if we are in a short position, exit it
		// here we don't call exit_short mainly because that would guarantee
		// 3 api calls, where as this only guarantees 3 if it needs to exit the short position
		if (info.shares() < 0.0)
		{
			if (!market_order(ticker, -info.shares())) return false;
			acct = get_account_info();
		}

		// calculating the amount of shares to order
		double available_bp = acct.buying_power() + acct.margin_used() - info.shares() * info.price();
		double shares_to_order = get_shares_to_order(available_bp, info.price(), info.minimum(),
			info.fee());

		// verifying that it doesn't attempt to buy more shares than allowed
		if (shares_to_order * info.price() * (1.0 + info.fee()) > available_bp) return false; 
		// exit if not ordering any
		if (shares_to_order == 0.0) return true;
		// calling in order
		return market_order(ticker, shares_to_order);
	}

	bool Client::exit_long(const std::string ticker, double pct)
	{
		AssetInfo info = get_asset_info(ticker);
		if (info.shares() <= 0.0) return true;
		return market_order(ticker, -info.shares());
	}

	bool Client::enter_short(const std::string ticker, double pct)
	{
		AccountInfo acct = get_account_info();
		AssetInfo info = get_asset_info(ticker);
		// if shorting is not allowed by either the account or the asset
		if (!_shorting_enabled || !acct.shorting_enabled()) return true;

		if (info.shares() > 0.0)
		{
			if (!market_order(ticker, -info.shares())) return false;
			acct = get_account_info();
		}

		double available_bp = (acct.buying_power() + acct.margin_used()) + info.shares() * info.price();
		double shares_to_order = get_shares_to_order(available_bp, info.price(), info.minimum(), info.fee());

		if (shares_to_order * info.price() * (1.0 * info.fee()) > available_bp) return false;
		if (shares_to_order == 0.0) return true;

		return market_order(ticker, -shares_to_order);
	}

	bool Client::exit_short(const std::string ticker, double pct)
	{
		AssetInfo info = get_asset_info(ticker);
		if (info.shares() >= 0.0) return true;
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

	bool Client::market_order(const std::string& ticker, double amount)
	{
		if (!func_ok("market_order", (void(*)())_market_order)) return false;
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
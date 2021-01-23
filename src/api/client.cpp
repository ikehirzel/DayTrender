#include "client.h"

#include <hirzel/strutil.h>
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define GET_ERROR_FUNC			"get_error"
#define MAX_CANDLES_FUNC		"max_candles"
#define INIT_FUNC				"init"
#define GET_CANDLES_FUNC		"get_candles"
#define MARKET_ORDER_FUNC		"marker_order"
#define TO_INTERVAL_FUNC		"to_interval"
#define GET_ACCT_INFO_FUNC		"get_account_info"
#define PAPER_MINIMUM_FUNC		"paper_minimum"
#define PAPER_FEE_FUNC			"paper_fee"
#define BACKTEST_INTERVALS_FUNC	"backtest_intervals"

namespace daytrender
{
	Client::Client(const std::string& _label, const std::string& _filepath,
		const std::vector<std::string>& _credentials, double _risk)
	{
		label = _label;
		risk = _risk;
		filename = hirzel::str::get_filename(_filepath);

		handle = new hirzel::Plugin(_filepath);

		if (!handle->is_lib_bound())
		{
			errorf("%s failed to bind: %s", filename, handle->get_error());
			return;
		}

		init_ptr = (void(*)(const std::vector<std::string>&))handle->bind_function(INIT_FUNC);
		if (!init_ptr) errorf(handle->get_error());

		get_candles_ptr = (void(*)(CandleSet&, const std::string&))handle->bind_function(GET_CANDLES_FUNC);
		if (!get_candles_ptr) errorf(handle->get_error());

		get_account_info_ptr = (void(*)(AccountInfo&))handle->bind_function(GET_ACCT_INFO_FUNC);
		if (!get_account_info_ptr) errorf(handle->get_error());

		market_order_ptr = (bool(*)(const std::string&, double))handle->bind_function("market_order");
		if (!market_order_ptr) errorf(handle->get_error());

		get_shares_ptr = (double(*)(const std::string&))handle->bind_function("get_shares");
		if (!get_shares_ptr) errorf(handle->get_error());

		close_all_positions_ptr = (bool(*)())handle->bind_function("close_all_positions");
		if (!close_all_positions_ptr) errorf(handle->get_error());

		market_open_ptr = (bool(*)())handle->bind_function("market_open");
		if (!market_open_ptr) errorf(handle->get_error());

		get_price_ptr = (double(*)(const std::string&))handle->bind_function("get_price");
		if (!get_price_ptr) errorf(handle->get_error());

		set_leverage_ptr = (bool(*)(int))handle->bind_function("set_leverage");
		if (!set_leverage_ptr) errorf(handle->get_error());

		get_leverage_ptr = (double(*)())handle->bind_function("get_leverage");
		if (!get_leverage_ptr) errorf(handle->get_error());

		// getters

		to_interval_ptr = (const char*(*)(int))handle->bind_function(TO_INTERVAL_FUNC);
		if (!to_interval_ptr) errorf(handle->get_error());

		max_candles_ptr = (int(*)())handle->bind_function(MAX_CANDLES_FUNC);
		if (!max_candles_ptr) errorf(handle->get_error());

		paper_fee_ptr = (double(*)())handle->bind_function(PAPER_FEE_FUNC);
		if (!paper_fee_ptr) errorf(handle->get_error());

		paper_minimum_ptr = (double(*)())handle->bind_function(PAPER_MINIMUM_FUNC);
		if (!paper_minimum_ptr) errorf(handle->get_error());

		backtest_intervals_ptr = (void(*)(std::vector<int>&))handle->bind_function(BACKTEST_INTERVALS_FUNC);
		if (!backtest_intervals_ptr) errorf(handle->get_error());

		get_error_ptr = (void(*)(std::string&))handle->bind_function(GET_ERROR_FUNC);
		if (!get_error_ptr) errorf(handle->get_error());

		bound = handle->all_bound();

		if (bound)
		{
			init_ptr(_credentials);
			successf("Successfully loaded %s client: '%s'", label, filename); 
		}
		else
		{
			errorf("Failed to bind to all functions of: '%s': %s", filename, handle->get_error());
		}
	}

	Client::~Client()
	{
		delete handle;
	}

	CandleSet Client::get_candles(const std::string& ticker, int interval, int max) const
	{
		printfmt("Getcandles: interval: %d, size: %d\n", interval, max);

		if (!get_candles_ptr)
		{
			errorf("%s: get_candles is not bound and cannot be executed!", filename);
			return CandleSet();
		}

		if (max > max_candles())
		{
			errorf("%s: candles requested is greater than maximum", filename);
			return CandleSet();
		}

		CandleSet candles(max, interval);
		get_candles_ptr(candles, ticker);

		std::cout << "client::get_candles: interval: " << candles.interval() << std::endl;

		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return candles;
	}

	AccountInfo Client::get_account_info() const
	{
		AccountInfo info;

		if (!get_account_info_ptr)
		{
			errorf("%s: get_account_info is not bound and cannot be executed!", filename);
			return info;
		}

		get_account_info_ptr(info);
		
		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return info;
	}

	bool Client::market_order(const std::string& ticker, double amount) const
	{
		if (!market_order_ptr)
		{
			errorf("%s: market_order is not bound and cannot be executed!", filename);
			return false;
		}

		bool res = market_order_ptr(ticker, amount);

		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return res;
	}

	double Client::get_shares(const std::string& ticker) const
	{
		if (!get_shares_ptr)
		{
			errorf("%s: get_shares is not bound and cannot be executed!", filename);
			return 0.0;
		}

		double shares = get_shares_ptr(ticker);

		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return shares;
	}

	bool Client::close_all_positions() const
	{
		if (!close_all_positions_ptr)
		{
			errorf("%s: close_all_positions is not bound and cannot be executed!", filename);
			return false;
		}

		bool res = close_all_positions_ptr();

		std::string error = get_error();
		if(!error.empty()) errorf("%s: %s", filename, error);
		return res;
	}

	bool Client::market_open() const
	{
		if (!market_open_ptr)
		{
			errorf("%s: paper_fee is not bound and cannot be executed!", filename);
			return false;
		}
		
		bool open = market_open_ptr();
		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return open;
	}

	double Client::get_price(const std::string& ticker) const
	{
		if (!get_price_ptr)
		{
			errorf("%s: get_price is not bound and cannot be executed!", filename);
			return false;
		}
		
		double price = get_price_ptr(ticker);
		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);
		
		return price;
	}

	double Client::get_leverage() const
	{
		if (!get_leverage_ptr)
		{
			errorf("%s: get_leverage is not bound and cannot be executed!", filename);
			return 0.0;
		}

		double leverage = get_leverage_ptr();
		std::string error = get_error();
		if (!error.empty()) errorf("%s: %s", filename, error);

		return leverage;
	}

	bool Client::set_leverage(int multiplier) const
	{
		if (!set_leverage_ptr)
		{
			errorf("%s: set_leverage is not bound and cannot be executed!", filename);
			return false;
		}

		bool success = set_leverage_ptr(multiplier);
		std::string error = get_error(); 
		if (!error.empty()) errorf("%s: %s", filename, error);

		return success;

	}

	/************************************
	 *		Getters for constants		*
	 ************************************/

	std::string Client::to_interval(int interval) const
	{
		if (!to_interval_ptr)
		{
			errorf("%s: to_interval is not bound and cannot be executed!", filename);
			return "";
		}

		return std::string(to_interval_ptr(interval));
	}

	double Client::paper_fee() const
	{
		if (!paper_fee_ptr)
		{
			errorf("%s: paper_fee is not bound and cannot be executed!", filename);
			return 0.0;
		}
	
		return paper_fee_ptr();
	}

	double Client::paper_minimum() const
	{
		if (!paper_minimum_ptr)
		{
			errorf("%s: paper_minimum is not bound and cannot be executed!", filename);
			return 0.0;
		}

		return paper_minimum_ptr();
	}

	std::vector<int> Client::backtest_intervals() const
	{
		if (!backtest_intervals_ptr)
		{
			errorf("%s: backtest_intervals is not bound and cannot be executed!", filename);
			return {};
		}

		std::vector<int> out;
		backtest_intervals_ptr(out);

		return out;
	}

	int Client::max_candles() const
	{
		if (!max_candles_ptr)
		{
			errorf("%s: max_candles is not bound and cannot be executed!", filename);
			return 0;
		}

		return max_candles_ptr();
	}

	std::string Client::get_error() const
	{
		if (!get_error_ptr)
		{
			errorf("%s: get_error is not bound and cannot be executed!", filename);
			return "";
		}
		
		std::string error;
		get_error_ptr(error);
		return error;
	}
}
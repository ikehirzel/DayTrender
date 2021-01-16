#include "tradeclient.h"

#include <hirzel/strutil.h>
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>

#define GET_CANDLES_FUNC	"get_candles"
#define MARKET_ORDER_FUNC	"marker_order"
#define TO_INTERVAL_FUNC	"to_interval"
#define GET_ACCT_INFO_FUNC	"get_account_info"
#define PAPER_MINIMUM_FUNC	"paper_minimum"
#define PAPER_FEE_FUNC		"paper_fee"

namespace daytrender
{
	TradeClient::TradeClient(const std::string& _label, const std::string& _filepath,
		const std::vector<std::string>& _credentials)
	{
		label = _label;
		filename = hirzel::str::get_filename(_filepath);
		credentials = _credentials;

		handle = new hirzel::Plugin(_filepath);

		get_candles_ptr =
		(void(*)(candleset&, const std::string&, int, int)) handle->bind_function(TO_INTERVAL_FUNC);

		get_account_info_ptr =
		(void(*)(account_info&)) handle->bind_function(GET_ACCT_INFO_FUNC);

		to_interval_ptr = (void(*)(std::string&, int)) handle->bind_function(GET_CANDLES_FUNC);

		paper_fee_ptr = (double(*)()) handle->bind_function("paper_fee");
		paper_minimum_ptr = (double(*)()) handle->bind_function("paper_minimum");

		bound = handle->all_bound();
	}

	std::string TradeClient::to_interval(int interval) const
	{
		std::string interval_str;

		if (to_interval_ptr)
		{
			to_interval_ptr(interval_str, interval);
		}
		else
		{
			errorf("%s: to_interval is not bound and cannot be executed!", filename);
		}


		return interval_str;
	}

	candleset TradeClient::get_candles(const std::string& ticker, int interval, int max) const
	{
		candleset candles;

		if (get_candles_ptr)
		{
			candles.init(max, interval);
			get_candles_ptr(candles, ticker, interval, max);
		}
		else
		{
			errorf("%s: get_candles is not bound and cannot be executed!", filename);
		}

		return candles;
	}

	account_info TradeClient::get_account_info() const
	{
		account_info info;

		if (get_account_info_ptr)
		{
			get_account_info_ptr(info);
		}
		else
		{
			errorf("%s: get_account_info is not bound and cannot be executed!", filename);
		}


		return info;
	}

	double TradeClient::paper_fee() const
	{
		if (paper_fee_ptr)
		{
			return paper_fee_ptr();
		}
		else
		{
			errorf("%s: paper_fee is not bound and cannot be executed!", filename);
			return 0.0;
		}
	}

	double TradeClient::paper_minimum() const
	{
		if (paper_minimum_ptr)
		{
			return paper_minimum_ptr();
		}
		else
		{
			errorf("%s: paper_minimum is not bound and cannot be executed!", filename);
			return 0.0;
		}
	}

	std::vector<int> TradeClient::backtest_intervals() const
	{
		std::vector<int> out;
		
		if (backtest_intervals_ptr)
		{
			backtest_intervals_ptr(out);
		}
		else
		{
			errorf("%s: backtest_intervals is not bound and cannot be executed!", filename);
		}

		return out;
	}
}
#pragma once

#ifndef PAPER_FEE
#error PAPER_FEE must be defined!
#endif

#ifndef PAPER_MINIMUM
#error PAPER_MINIMUM must be defined!
#endif

#ifndef BACKTEST_INTERVALS
#error BACKTEST_INTERVALS must be defined!
#endif

#ifndef MAX_CANDLES
#error MAX_CANDLES must be defined!
#endif

#include <string>
#include <vector>
#include <candle.h>
#include <interval.h>
#include <clienttypes.h>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace daytrender;
using namespace httplib;
using nlohmann::json;

std::string error;

#define JSON_FORMAT "application/json"

bool res_ok(const Result& res)
{
	if (!res)
	{
		error = "failed to get a response";
		return false;
	}
	if (res->status < 200 || res->status > 299)
	{
		error = "response status was not okay: " + std::to_string(res->status);
		if (!res->body.empty())
		{
			error += ": " + res->body;
		}
		return false;
	}
	if (res->body.empty())
	{
		error = "response body was empty";
		return false;
	}

	return true;
}

extern "C"
{
	void init(const std::vector<std::string>& credentials);
	void get_candles(CandleSet& candles, const std::string& ticker);
	void get_account_info(AccountInfo& info);
	bool market_order(const std::string& ticker, double amount);
	double get_shares(const std::string& ticker);
	bool close_all_positions();
	bool market_open();
	double get_price(const std::string& ticker);

	const char* to_interval(int interval);
	void backtest_intervals(std::vector<int>& out)
	{
		out = { BACKTEST_INTERVALS };
	}

	double paper_fee()
	{
		return PAPER_FEE;
	}

	double paper_minimum()
	{
		return PAPER_MINIMUM;
	}

	int max_candles()
	{
		return MAX_CANDLES;
	}

	void get_error(std::string& out)
	{
		out = error;
		error.clear();
	}
}
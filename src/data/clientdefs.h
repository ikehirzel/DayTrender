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

#include "candle.h"
#include "interval.h"
#include "accountinfo.h"

#include <string>
#include <vector>

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace daytrender;
using namespace httplib;
using nlohmann::json;

#define JSON_FORMAT "application/json"

std::string error;

bool res_ok(const Result& res)
{
	if (!res)
	{
		error = "failed to get a response";
		return false;
	}
	else if (res->status < 200 || res->status > 299)
	{
		error = "response status was not okay: " + std::to_string(res->status);
		if (!res->body.empty())
		{
			error += ": " + res->body;
		}
		return false;
	}
	else if (res->body.empty())
	{
		error = "response body was empty";
		return false;
	}

	return true;
}

extern "C"
{
	// non-returning functions

	bool init(const std::vector<std::string>& credentials);
	bool market_order(const std::string& ticker, double amount);\
	bool close_all_positions();
	bool set_leverage(int numerator);

	// returning functions

	bool get_candles(CandleSet& candles, const std::string& ticker);
	bool get_account_info(AccountInfo& info);
	bool market_open(bool&);
	bool get_shares(double&, const std::string& ticker);
	bool get_price(double&, const std::string& ticker);
	bool to_interval(const char*& interval_str, int interval);

	// const functions

	void backtest_intervals(std::vector<int>& out) { out = { BACKTEST_INTERVALS }; }
	double paper_fee() { return PAPER_FEE; }
	double paper_minimum() { return PAPER_MINIMUM; }
	int max_candles() { return MAX_CANDLES; }

	void get_error(std::string& out)
	{
		out = error;
		error.clear();
	}
}
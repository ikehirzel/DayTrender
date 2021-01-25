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
#include <chrono>
#include "candle.h"
#include "interval.h"
#include "accountinfo.h"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include <httplib.h>
#include <nlohmann/json.hpp>

using namespace daytrender;
using namespace httplib;
using nlohmann::json;

double _max_loss = 0.05;
double _equity = 0.0;
long long last_update = 0;
std::string error;

#define JSON_FORMAT "application/json"

// updates equity and returns false if 
bool update_equity(double equity)
{
	bool res = true;

	// if the current level of loss is more than 5 percent of equity
	if (equity - _equity <= _equity * -_max_loss)
	{
		res = false;
	}

	long long time = std::chrono::duration_cast<std::chrono::seconds>
				(std::chrono::system_clock::now().time_since_epoch()).count();

	if ((time - last_update) > DAY)
	{
		_equity = equity;
		last_update = time;
	}

	return res;
}

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

	void set_max_loss(double max_loss)
	{
		_max_loss = max_loss;
	}

	void get_error(std::string& out)
	{
		out = error;
		error.clear();
	}
}
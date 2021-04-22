#define KEY_COUNT 2
#define BACKTEST_INTERVALS MIN1, MIN5, MIN15, HOUR1
#define MAX_CANDLES 5000

#include <iostream>
#include <clientdefs.h>
#include <ctime>

std::string accountid, token;

httplib::SSLClient client("api-fxpractice.oanda.com");

bool init(const std::vector<std::string>& credentials)
{
	accountid = credentials[0];
	token = credentials[1];
	client.set_bearer_token_auth(token.c_str());
	return true;
}

bool get_candles(CandleSet& candles, const std::string& ticker)
{
	std::string url = "/v3/instruments/" + ticker + "/candles";
	
	const char* interval_str = to_interval(candles.interval());

	if (!interval_str)
	{
		error = "interval given (" + std::to_string(candles.interval()) + ") is not valid";
		return false;
	}
	
	Params p = {
		{ "granularity", interval_str },
		{ "count", std::to_string(candles.size()) }
	};

	url += '?' + detail::params_to_query_str(p);
	auto res = client.Get(url.c_str());

	if (res_ok(res))
	{
		Data json = Data::parse_json(res->body);
		if (json.is_error())
		{
			error = json.to_string();
			return false;
		}

		const Data& candles_json = json["candles"];

		if (candles_json.empty() || !candles_json.is_array())
		{
			error = "no candles were received";
			return false;
		}

		else if (candles_json.size() != candles.size())
		{
			error = "not all candles were received";
			return false;
		}

		for (int i = 0; i < candles_json.size(); i++)
		{
			const Data& candle = candles_json[i];
			const Data& mid = candle["mid"];

			candles.set(i) =
			{
				candle["o"].to_double(),
				candle["h"].to_double(),
				candle["l"].to_double(),
				candle["c"].to_double(),
				candle["v"].to_double()
			};
		}

		return true;
	}

	return false;
}

bool get_account_info(AccountInfo& info)
{
	std::string url = "/v3/accounts/" + accountid + "/summary";
	auto res = client.Get(url.c_str());

	if (res_ok(res))
	{
		Data json = Data::parse_json(res->body);
		if (json.is_error())
		{
			error = json.to_string();
			return false;
		}
		const Data& acc = json["account"];
		double balance = acc["balance"].to_double();
		double margin_rate = acc["marginRate"].to_double();
		double buying_power = acc["marginAvailable"].to_double() / margin_rate;
		double margin_used = acc["marginUsed"].to_double();
		double equity = acc["NAV"].to_double();
		int leverage = (int)(1.0 / margin_rate);
		bool shorting_enabled = true;
		info = { balance, buying_power, margin_used, equity, leverage, shorting_enabled };
		return true;
	}

	return false;
}

bool market_order(const std::string& ticker, double amount)
{
	std::string url = "/v3/accounts/" + accountid + "/orders";

	Data req;
	req["order"] = Data::Table({
		{ "type", "MARKET" },
		{ "instrument", ticker },
		{ "units", std::to_string(amount) }
	});

	auto res = client.Post(url.c_str(), req.to_json(), JSON_FORMAT);
	if (!res_ok(res)) return false;
	Data json = Data::parse_json(res->body);

	if (json.is_error())
	{
		error = json.to_string();
		return false;
	}

	if (json["orderCreateTransaction"].is_null())
	{
		error = "order was not created correctly";
		return false;
	}

	if (json["orderCancelTransaction"].is_null())
	{
		error = "order was cancelled";
		return false;
	}

	if (json["orderFillTransaction"].is_null())
	{
		error = "order was not fulfilled";
		return false;
	}

	return true;
}

bool get_asset_info(AssetInfo& info, const std::string& ticker)
{
	// getting share count
	std::string url = "/v3/accounts/" + accountid + "/positions/" + ticker;
	auto res = client.Get(url.c_str());
	if (!res_ok(res)) return false;

	Data json = Data::parse_json(res->body);
	if (json.is_error())
	{
		error = json.to_string();
		return false;
	}
	const Data& long_json = json["position"]["long"];
	const Data& short_json = json["position"]["short"];
	
	double shares = short_json["units"].to_double() + long_json["units"].to_double();
	double amt_invested = 0.0;

	// long position
	if (shares > 0.0)
	{
		amt_invested = shares * long_json["averagePrice"].to_double();
	}
	// short position
	else if (shares < 0.0)
	{
		amt_invested = -shares * short_json["averagePrice"].to_double();
	}

	// getting fee and price
	url = "/v3/instruments/" + ticker + "/candles?count=5000&granularity=S5&price=BAM";
	res = client.Get(url.c_str());
	if (!res_ok(res)) return false;
	
	json = Data::parse_json(res->body);
	if (json.is_error())
	{
		error = json.to_string();
		return false;
	}

	
	auto candles = json["candles"].to_array();
	double price = candles.back()["mid"]["c"].to_double();

	// calculating half spread cost as a percentage
	double fee = 0.0;
	for (const Data& c : candles)
	{
		fee += (c["ask"]["c"].to_double() - c["bid"]["c"].to_double());
	}
	fee /= ((double)candles.size() * 2.0);
	fee /= price;

	// constructing info
	info = { amt_invested, fee, 1.0, price, shares };
	
	return true;
}

bool set_leverage(int multiplier)
{
	std::string url = "/v3/accounts/" + accountid + "/configuration";
	client.Patch(url.c_str());
	if (multiplier > 50)
	{
		error = "multiplier (" + std::to_string(multiplier) + " was over maximum (50) and was capped.";
		multiplier = 50;
	}
	else if (multiplier < 1)
	{
		error = "multiplier (" + std::to_string(multiplier) + " was below minimum (1) and was capped .";
		multiplier = 1;
	}
	Data req;
	req["marginRate"] = std::to_string(1.0 / (double)multiplier);
	auto res = client.Patch(url.c_str(), req.to_json(), JSON_FORMAT);
	
	return res_ok(res);
}

bool close_all_positions()
{
	std::string url =  "/v3/accounts/" + accountid + "/positions";
	auto res = client.Get(url.c_str());

	if (res_ok(res))
	{
		std::string error_glob;
		std::vector<std::string> failed_tickers;
		Data json = Data::parse_json(res->body);
		if (json.is_error())
		{
			error = json.to_string();
			return false;
		}
		auto positions = json["positions"].to_array();
		for (const Data& p : positions)
		{
			// getting total units
			double shares = p["long"]["units"].to_double() + p["short"]["units"].to_double();

			// if the position is still open, close it
			if (shares != 0.0)
			{
				// get ticker
				std::string ticker = p["instrument"].to_string();
				// if failed, log the error 
				if (!market_order(ticker, -shares))
				{
					error_glob += ticker + ": " + error + ". ";
					failed_tickers.push_back(ticker);
				}
			}
		}

		// globbing all errors from trying to close all positions
		if (!failed_tickers.empty())
		{
			error = "failed to close assets: ";
			for (int i = 0; i < failed_tickers.size(); i++)
			{
				if (i > 0)
				{
					error += ", ";
				}
				error += failed_tickers[i];
			}
			error += " ::: " + error_glob;
			return false;
		}
		return true;
	}

	return false;
}

bool secs_till_market_close(int& seconds)
{
	seconds = 0;
	time_t time_since_epoch = time(NULL);
	tm* curr_time = gmtime(&time_since_epoch);

	int second = curr_time->tm_sec;
	int minute = curr_time->tm_min;
	int hour = curr_time->tm_hour;
	int weekday = curr_time->tm_wday;
	int month = curr_time->tm_mon;
	int monthday = curr_time->tm_mday;

	// if is between christmas eve and jan 1
	if ((month == 0 && monthday == 1) || (month == 11 && monthday >= 24)) return true;
	// if is market weekend
	if (weekday == 6 || (weekday == 5 && hour >= 22) || (weekday == 0 && hour < 22)) return true;

	// every calculation following is needs to find time till friday at 10pm
	int days_till_close = 5 - weekday;
	int hours_till_close = (22 - hour) + days_till_close * 24;
	int mins_till_close = -minute + hours_till_close * 60;
	seconds = -second + mins_till_close * 60;
	return true;
}

const char* to_interval(int interval)
{
	switch(interval)
	{
	case MIN1:		return "M1";
	case MIN2:		return "M2";
	case MIN4:		return "M4";
	case MIN5:		return "M5";
	case MIN10:		return "M10";
	case MIN15:		return "M15";
	case MIN30:		return "M30";
	case HOUR1:		return "H1";
	case HOUR2:		return "H2";
	case HOUR3:		return "H3";
	case HOUR4:		return "H4";
	case HOUR6:		return "H6";
	case HOUR8:		return "H8";
	case HOUR12:	return "H12";
	case DAY:		return "D";
	case WEEK:		return "W";
	case MONTH:		return "M";
	}
	return nullptr;
}
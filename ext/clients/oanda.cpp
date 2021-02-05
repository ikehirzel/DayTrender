#define FEE 0.00007
#define ORDER_MINIMUM 1.0
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
	
	const char* interval_str = nullptr;

	if (!to_interval(interval_str, candles.interval()))
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
		json jres = json::parse(res->body);
		json& candles_json = jres["candles"];

		if (candles_json.empty())
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
			json& candle_json = candles_json[i];
			json& mid = candle_json["mid"];

			double o = std::stod(mid["o"].get<std::string>());
			double h = std::stod(mid["h"].get<std::string>());
			double l = std::stod(mid["l"].get<std::string>());
			double c = std::stod(mid["c"].get<std::string>());
			double v = candle_json["volume"].get<double>();
			candles.set(i) = { o, h, l, c, v };
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
		std::cout << res->body << "\n\n";
		json res_json = json::parse(res->body);
		json& acc = res_json["account"];
		double balance = std::stod(acc["balance"].get<std::string>());
		double margin_rate = std::stod(acc["marginRate"].get<std::string>());
		double buying_power = std::stod(acc["marginAvailable"].get<std::string>()) / margin_rate;
		double equity = std::stod(acc["NAV"].get<std::string>());
		int leverage = (int)(1.0 / margin_rate);
		bool shorting_enabled = true;
		info = { balance, buying_power, equity, leverage, shorting_enabled };
		return true;
	}

	return false;
}

bool market_order(const std::string& ticker, double amount)
{
	std::string url = "/v3/accounts/" + accountid + "/orders";
	json req;
	json& order = req["order"];
	order["type"] = "MARKET";
	order["instrument"] = ticker;
	order["units"] = std::to_string(amount);

	auto res = client.Post(url.c_str(), req.dump(), JSON_FORMAT);
	if (res_ok(res))
	{
		json res_json = json::parse(res->body);

		if (res_json["orderCreateTransaction"].is_null())
		{
			error = "order was not created correctly";
			return false;
		}

		if (!res_json["orderCancelTransaction"].is_null())
		{
			error = "order was canceled";
			return false;
		}

		if (res_json["orderFillTransaction"].is_null())
		{
			error = "order was not fulfilled";
			return false;
		}

		return true;
	}
	return false;
}

bool get_shares(double& shares, const std::string& ticker)
{
	std::string url = "/v3/accounts/" + accountid + "/positions/" + ticker;
	auto res = client.Get(url.c_str());
	if (res_ok(res))
	{
		json res_json = json::parse(res->body);
		double longu = std::stod(res_json["position"]["long"]["units"].get<std::string>());
		double shortu = std::stod(res_json["position"]["short"]["units"].get<std::string>());
		
		shares = longu + shortu;
		return true;
	}
	return false;
}

bool get_price(double& price, const std::string& ticker)
{
	std::string url = "/v3/instruments/" + ticker + "/candles?count=1&granularity=S5";
	auto res = client.Get(url.c_str());
	if (res_ok(res))
	{
		json res_json = json::parse(res->body);
		price = std::stod(res_json["candles"][0]["mid"]["c"].get<std::string>());
		return true;
	}
	return false;
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
	json req_json;
	req_json["marginRate"] = (1.0 / (double)multiplier);
	auto res = client.Patch(url.c_str(), req_json.dump(), JSON_FORMAT);
	
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
		json res_json = json::parse(res->body);
		json& pos_json = res_json["positions"];
		for (int i = 0; i < pos_json.size(); i++)
		{
			json& position = pos_json[i];

			// getting total units
			double longu = std::stod(position["long"]["units"].get<std::string>());
			double shortu = std::stod(position["short"]["units"].get<std::string>());
			double shares = longu + shortu;

			// if the position is still open
			if (shares != 0.0)
			{
				// get rid of the shares
				std::string ticker = position["instrument"].get<std::string>();
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

bool to_interval(const char*& interval_str, int interval)
{
	switch(interval)
	{
		case MIN1:
			interval_str = "M1";
			return true;

		case MIN2:
			interval_str = "M2";
			return true;

		case MIN4:
			interval_str = "M4";
			return true;

		case MIN5:
			interval_str = "M5";
			return true;

		case MIN10:
			interval_str = "M10";
			return true;

		case MIN15:
			interval_str = "M15";
			return true;

		case MIN30:
			interval_str = "M30";
			return true;

		case HOUR1:
			interval_str = "H1";
			return true;

		case HOUR2:
			interval_str = "H2";
			return true;

		case HOUR3:
			interval_str = "H3";
			return true;

		case HOUR4:
			interval_str = "H4";
			return true;

		case HOUR6:
			interval_str = "H6";
			return true;

		case HOUR8:
			interval_str = "H8";
			return true;

		case HOUR12:
			interval_str = "H12";
			return true;

		case DAY:
			interval_str = "D";
			return true;

		case WEEK:
			interval_str = "W";
			return true;

		case MONTH:
			interval_str = "M";
			return true;
	}
	return false;
}
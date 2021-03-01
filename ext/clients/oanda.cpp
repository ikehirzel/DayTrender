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
		JsonValue json_val;
		JsonObject jres;
		picojson::parse(json_val, res->body);
		const JsonArray& candles_json = jres.at("candles").get<JsonArray>();

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
			const JsonObject& candle_json = candles_json.at(i).get<JsonObject>();
			const JsonObject& mid = candle_json.at("mid").get<JsonObject>();

			double o = std::stod(mid.at("o").get<std::string>());
			double h = std::stod(mid.at("h").get<std::string>());
			double l = std::stod(mid.at("l").get<std::string>());
			double c = std::stod(mid.at("c").get<std::string>());
			double v = candle_json.at("volume").get<double>();
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
		//std::cout << res->body << "\n\n";
		JsonValue json_val;
		std::string err = picojson::parse(json_val, res->body);
		if (!err.empty())
		{
			std::cout << "Error: " << err << std::endl;
			return false;
		}
		const JsonObject& res_json = json_val.get<JsonObject>();
		const JsonObject& acc = res_json.at("account").get<JsonObject>();
		double balance = std::stod(acc.at("balance").get<std::string>());
		double margin_rate = std::stod(acc.at("marginRate").get<std::string>());
		double buying_power = std::stod(acc.at("marginAvailable").get<std::string>()) / margin_rate;
		double margin_used = std::stod(acc.at("marginUsed").get<std::string>());
		double equity = std::stod(acc.at("NAV").get<std::string>());
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

	JsonValue req;
	JsonObject& order = req.get<JsonObject>()["order"].get<JsonObject>();
	
	order["type"] = JsonValue("MARKET");
	order["instrument"] = JsonValue(ticker);
	order["units"] = JsonValue(std::to_string(amount));

	auto res = client.Post(url.c_str(), req.serialize(), JSON_FORMAT);
	if (res_ok(res))
	{
		JsonValue json;
		std::string err = picojson::parse(json, res->body);
		const JsonObject& res = json.get<JsonObject>();

		if (!err.empty())
		{
			std::cout << "Oanda Client: " << err << std::endl;
			return false;
		}

		if (res.at("orderCreateTransaction").is<picojson::null>())
		{
			error = "order was not created correctly";
			return false;
		}

		if (!res.at("orderCancelTransaction").is<picojson::null>())
		{
			error = "order was canceled";
			return false;
		}

		if (res.at("orderFillTransaction").is<picojson::null>())
		{
			error = "order was not fulfilled";
			return false;
		}

		return true;
	}
	return false;
}

bool get_asset_info(AssetInfo& info, const std::string& ticker)
{
	// getting share count
	std::string url = "/v3/accounts/" + accountid + "/positions/" + ticker;
	auto response = client.Get(url.c_str());
	if (!res_ok(response)) return false;

	JsonValue json;
	std::string err = picojson::parse(json, response->body);
	if (!err.empty())
	{
		std::cout << "Oanda Client: " << err << std::endl;
		return false;
	}
	const JsonObject& res = json.get<JsonObject>();
	const JsonObject& long_json = res.at("position").get<JsonObject>().at("long").get<JsonObject>();
	const JsonObject& short_json = res.at("position").get<JsonObject>().at("short").get<JsonObject>();
	double longu = std::stod(long_json.at("units").get<std::string>());
	double shortu = std::stod(short_json.at("units").get<std::string>());

	if (longu > 0.0 && shortu < 0.0)
	{
		error = "shares were both negative and positive!";
		return false;
	}
	
	double shares = longu + shortu;	
	double amt_invested = 0.0;

	// long position
	if (longu > 0.0)
	{
		double avg_price = std::stod(long_json.at("averagePrice").get<std::string>());
		amt_invested = longu * avg_price;
	}
	// short position
	else if (shortu < 0.0)
	{
		double avg_price = std::stod(short_json.at("averagePrice").get<std::string>());
		amt_invested = -shortu * avg_price;
	}

	// getting fee and price
	url = "/v3/instruments/" + ticker + "/candles?count=5000&granularity=S5&price=BAM";
	response = client.Get(url.c_str());
	if (!res_ok(response)) return false;
	
	err = picojson::parse(json, response->body);
	if (!err.empty())
	{
		std::cout << "Oanda Client: " << err << std::endl;
		return false;
	}
	const JsonArray& candles_json = res.at("candles").get<JsonArray>();
	const JsonObject& last_json = candles_json.back().get<JsonObject>();
	double price = std::stod(last_json.at("mid").get<JsonObject>().at("c").get<std::string>());

	// calculating half spread cost as a percentage
	double fee = 0.0;
	for (const JsonValue& candle : candles_json)
	{

		double ask = std::stod(candle.get<JsonObject>().at("ask").get<JsonObject>().at("c").get<std::string>());
		double bid = std::stod(candle.get<JsonObject>().at("bid").get<JsonObject>().at("c").get<std::string>());
		fee += ask - bid;
	}
	fee /= ((double)candles_json.size() * 2.0);
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
	JsonValue req;
	req.get<JsonObject>()["marginRate"] = JsonValue(1.0 / (double)multiplier);
	auto res = client.Patch(url.c_str(), req.serialize(), JSON_FORMAT);
	
	return res_ok(res);
}

bool close_all_positions()
{
	std::string url =  "/v3/accounts/" + accountid + "/positions";
	auto response = client.Get(url.c_str());

	if (res_ok(response))
	{
		std::string error_glob;
		std::vector<std::string> failed_tickers;
		JsonValue res;
		std::string err = picojson::parse(res, response->body);
		if (!err.empty())
		{
			std::cout << "Oanda Client: " << err << std::endl;
		}
		const JsonArray& pos_json = res.get<JsonObject>().at("positions").get<JsonArray>();
		for (int i = 0; i < pos_json.size(); i++)
		{
			const JsonObject& position = pos_json[i].get<JsonObject>();

			// getting total units
			double longu = std::stod(position.at("long").get<JsonObject>().at("units").get<std::string>());
			double shortu = std::stod(position.at("short").get<JsonObject>().at("units").get<std::string>());
			double shares = longu + shortu;

			// if the position is still open
			if (shares != 0.0)
			{
				// get rid of the shares
				std::string ticker = position.at("instrument").get<std::string>();
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
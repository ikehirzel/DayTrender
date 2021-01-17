#define PAPER_FEE 69.69
#define PAPER_MINIMUM 5390
#define BACKTEST_INTERVALS MIN1, MIN15, HOUR1
#define MAX_CANDLES 5000

#include <iostream>
#include <clientdefs.h>

std::string username, accountid, token;

httplib::SSLClient client("api-fxpractice.oanda.com");

void init(const std::vector<std::string>& credentials)
{
	username = credentials[0];
	accountid = credentials[1];
	token = credentials[2];
	//client.set_default_headers({{ "Content-Type", "application/json" }});
	client.set_bearer_token_auth(token.c_str());
}

void get_candles(candleset& candles, const std::string& ticker)
{
	std::string url = "/v3/accounts/" + accountid + "/instruments/" + ticker + "/candles";

	Params p = {
		{ "granularity", to_interval(candles.interval) },
		{ "count", std::to_string(candles.size) }
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
			return;
		}
		else if (candles_json.size() != candles.size)
		{
			error = "not all candles were received";
			return;
		}

		for (int i = 0; i < candles_json.size(); i++)
		{
			json& c = candles_json[i];
			json& mid = c["mid"];

			candles[i].open = std::stod(mid["o"].get<std::string>());
			candles[i].high = std::stod(mid["h"].get<std::string>());
			candles[i].low = std::stod(mid["l"].get<std::string>());
			candles[i].close = std::stod(mid["c"].get<std::string>());
			candles[i].volume = c["volume"].get<double>();
		}
	}
}

void get_account_info(account_info& info)
{
	std::string url = "/v3/accounts/" + accountid;
	auto res = client.Get(url.c_str());

	if (res_ok(res))
	{
		json res_json = json::parse(res->body);
		json& acc = res_json["account"];
		info.balance = std::stod(acc["balance"].get<std::string>());
		info.buying_power = std::stod(acc["marginAvailable"].get<std::string>());
		info.equity = std::stod(acc["NAV"].get<std::string>());
	}
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

double get_shares(const std::string& ticker)
{
	std::string url = "/v3/accounts/" + accountid + "/positions/" + ticker;
	auto res = client.Get(url.c_str());
	if (res_ok(res))
	{
		json res_json = json::parse(res->body);
		double longu = std::stod(res_json["position"]["long"]["units"].get<std::string>());
		double shortu = std::stod(res_json["position"]["short"]["units"].get<std::string>());
		
		return longu + shortu;
	}
	return 0.0;
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

const char* to_interval(int interval)
{
	switch(interval)
	{
		case MIN1:
			return "M1";
		case MIN2:
			return "M2";
		case MIN4:
			return "M4";
		case MIN5:
			return "M5";
		case MIN10:
			return "M10";
		case MIN15:
			return "M15";
		case MIN30:
			return "M30";
		case HOUR1:
			return "H1";
		case HOUR2:
			return "H2";
		case HOUR3:
			return "H3";
		case HOUR4:
			return "H4";
		case HOUR6:
			return "H6";
		case HOUR8:
			return "H8";
		case HOUR12:
			return "H12";
		case DAY:
			return "D";
		case WEEK:
			return "W";
		case MONTH:
			return "M";
		default:
			return "M1";
	}
}
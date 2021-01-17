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
		std::cout << "Acc:\n" << acc.dump() << std::endl;
		info.balance = std::stod(acc["balance"].get<std::string>());
		info.buying_power = info.balance + std::stod(acc["marginAvailable"].get<std::string>());
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
	std::cout << "The request: " << req.dump() << std::endl;
	auto res = client.Post(url.c_str(), req.dump(), JSON_FORMAT);
	if (res_ok(res))
	{
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
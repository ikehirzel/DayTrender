#define KEY_COUNT 2
#define MAX_CANDLES 5000

#include <iostream>
#include <api/client_api.h>
#include <ctime>

std::string accountid, token;

httplib::SSLClient client("api-fxpractice.oanda.com");

const char *init(const char** credentials)
{
	accountid = credentials[0];
	token = credentials[1];
	client.set_bearer_token_auth(token.c_str());
	return NULL;
}

const char *get_price_history(PriceHistory* out, const char *ticker)
{
	std::string url = "/v3/instruments/" + std::string(ticker) + "/candles";
	PriceHistory& hist = *out;
	const char* interval_str = to_interval(hist.interval());

	if (!interval_str)
	{
		return "interval given is not valid";
	}
	
	httplib::Params p = {
		{ "granularity", interval_str },
		{ "count", std::to_string(hist.size()) }
	};

	url += '?' + httplib::detail::params_to_query_str(p);
	
	auto res = client.Get(url.c_str());

	// if there was an error, return it
	const char *err = res_err(res);
	if (err) return err;

	Data json = Data::parse_json(res->body);
	if (json.is_error())
	{
		return "json failed to parse";
	}

	const Data& candles_json = json["candles"];

	if (candles_json.empty() || !candles_json.is_array())
	{
		return "no candles were received";
	}
	else if (candles_json.size() != hist.size())
	{
		return "not all candles were received";
	}

	for (int i = 0; i < hist.size(); i++)
	{
		const Data& candle = candles_json[i];
		const Data& mid = candle["mid"];
		
		hist[i] =
		{
			mid["o"].to_double(),
			mid["h"].to_double(),
			mid["l"].to_double(),
			mid["c"].to_double(),
			candle["volume"].to_double()
		};
	}

	return NULL;
}

const char *get_account(Account *out)
{
	std::string url = "/v3/accounts/" + accountid + "/summary";
	auto res = client.Get(url.c_str());

	const char *err = res_err(res);
	if (err) return err;

	Data json = Data::parse_json(res->body);

	if (json.is_error())
	{
		return "json failed to parse";
	}

	const Data& acc = json["account"];

	double margin_rate = acc["marginRate"].to_double();

	*out =
	{
		acc["balance"].to_double(),
		acc["marginAvailable"].to_double() / margin_rate,
		acc["marginUsed"].to_double(),
		acc["NAV"].to_double(),
		(int)(1.0 / margin_rate),
		true
	};

	return NULL;
}

const char *market_order(const char *ticker, double amount)
{
	std::string url = "/v3/accounts/" + accountid + "/orders";

	Data req;
	req["order"] = Data::Table({
		{ "type", "MARKET" },
		{ "instrument", ticker },
		{ "units", std::to_string(amount) }
	});

	auto res = client.Post(url.c_str(), req.to_json(), JSON_FORMAT);

	// if error exit
	const char *error = res_err(res);
	if (error) return error;

	Data json = Data::parse_json(res->body);

	if (json.is_error())
	{
		error = "json failed to parse";
	}

	if (json["orderCreateTransaction"].is_null())
	{
		return "order was not created correctly";
	}

	if (json["orderCancelTransaction"].is_null())
	{
		return "order was cancelled";
	}

	if (json["orderFillTransaction"].is_null())
	{
		return "order was not fulfilled";
	}

	return NULL;
}

const char *get_position(Position* out, const char *ticker)
{
	// getting share count
	std::string url = "/v3/accounts/" + accountid + "/positions/" + ticker;
	auto res = client.Get(url.c_str());

	const char *error = res_err(res);
	if (error) return error;

	Data json = Data::parse_json(res->body);
	if (json.is_error())
	{
		return "json failed to parse";
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
	url = "/v3/instruments/" + std::string(ticker) + "/candles?count=20&granularity=S5&price=BAM";
	res = client.Get(url.c_str());

	// exit if error
	error = res_err(res);
	if (error) return error;
	
	
	json = Data::parse_json(res->body);
	if (json.is_error())
	{
		return "json failed to parse";
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
	*out = { amt_invested, fee, 1.0, price, shares };
	
	return NULL;
}

const char *set_leverage(uint32_t multiplier)
{
	std::string url = "/v3/accounts/" + accountid + "/configuration";
	client.Patch(url.c_str());
	if (multiplier > 50)
	{
		return "leverage higher than maximum (50) is not allowed";
	}
	else if (multiplier == 0)
	{
		return "leverage of 0 is not allowed";
	}
	Data req;
	req["marginRate"] = std::to_string(1.0 / (double)multiplier);
	auto res = client.Patch(url.c_str(), req.to_json(), JSON_FORMAT);
	
	const char *error = res_err(res);
	if (error) return error;

	return NULL;
}

const char *close_all_positions()
{
	std::string url =  "/v3/accounts/" + accountid + "/positions";
	auto res = client.Get(url.c_str());

	// exit if error
	const char *error = res_err(res);
	if (error) return error;

	std::string error_glob;
	std::vector<std::string> failed_tickers;

	Data json = Data::parse_json(res->body);
	if (json.is_error()) return "json failed to parse";

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
			error = market_order(ticker.c_str(), -shares);
		}
	}

	// globbing all errors from trying to close all positions
	if (!failed_tickers.empty())
	{
		return "failed to close all assets";
	}
	return error;
}

uint32_t secs_till_market_close()
{
	uint32_t seconds = 0;
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
	return seconds;
}

const char* to_interval(uint32_t interval)
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
	default:		return nullptr;
	}
}
#include "oandaclient.h"

#include "../data/interval.h"
#include <hirzel/fountain.h>

namespace daytrender
{
	OandaClient::OandaClient(const std::vector<std::string>& credentials) : TradeClient(credentials)
	{
		this->username = this->credentials[0];
		this->accountid = "/" + this->credentials[1];
		this->token = this->credentials[2];
		client = new RestClient(OANDA_BASE_URL, {
			{ "Content-Type", "application/json" },
		});
		client->set_bearer_token(token);
	}

	OandaClient::~OandaClient()
	{
		delete client;
	}

	double OandaClient::getBalance()
	{
		json res = client->get(OANDA_ACCOUNTS + accountid);
		std::string balance_str =  res["account"]["balance"].get<std::string>();
		return stod(balance_str);
	}

	bool OandaClient::setMargin(double marginRate)
	{
		std::string url = OANDA_ACCOUNTS + accountid + OANDA_CONFIG;

		json params;
		params["marginRate"] = marginRate;

		json res = client->jsonbody_request(METHOD_PATCH, url, params);

		if(res["clientConfigureTransaction"].size())
		{
			return true;
		}

		return false;
	}

	bool OandaClient::marketOrder(const std::string& pair, double units)
	{
		std::string url = OANDA_ACCOUNTS + accountid + OANDA_ORDERS;
		json params;

		params["type"] = "MARKET";
		params["instrument"] = pair;
		params["units"] = units;

		json res = client->post(url, params);

		if(res["createOrderTransaction"].size())
		{
			return true;
		}
		return false;
	}

	std::string OandaClient::getAccounts()
	{
		json res = client->get(OANDA_ACCOUNTS);
		return res.dump();
	}

	std::string OandaClient::getPositions()
	{
		std::string url = OANDA_ACCOUNTS + accountid;
		json res = client->get(url);
		return res["account"]["positions"].dump();
	}

	std::string OandaClient::toInterval(unsigned int interval)
	{
		switch(interval)
		{
			case SEC5:
				return OANDA_5_SEC;
			case SEC10:
				return OANDA_10_SEC;
			case SEC15:
				return OANDA_15_SEC;
			case SEC30:
				return OANDA_30_SEC;
			case MIN1:
				return OANDA_1_MIN;
			case MIN2:
				return OANDA_2_MIN;
			case MIN4:
				return OANDA_4_MIN;
			case MIN5:
				return OANDA_5_MIN;
			case MIN10:
				return OANDA_10_MIN;
			case MIN15:
				return OANDA_15_MIN;
			case MIN30:
				return OANDA_30_MIN;
			case HOUR1:
				return OANDA_1_HOUR;
			case HOUR2:
				return OANDA_2_HOUR;
			case HOUR3:
				return OANDA_3_HOUR;
			case HOUR4:
				return OANDA_4_HOUR;
			case HOUR6:
				return OANDA_6_HOUR;
			case HOUR8:
				return OANDA_8_HOUR;
			case HOUR12:
				return OANDA_12_HOUR;
			case DAY:
				return OANDA_1_DAY;
			case WEEK:
				return OANDA_1_WEEK;
			case MONTH:
				return OANDA_1_MONTH;
			default:
				return OANDA_1_MIN;
		}
	}

	candleset OandaClient::getCandles(const std::string& ticker, unsigned int interval, unsigned int max)
	{
		max = (max > 0) ? max : OANDA_MAX_CANDLES;
		std::vector<candle> candles;
		std::string url = OANDA_ACCOUNTS + accountid + OANDA_INSTRUMENTS + "/" + ticker + OANDA_CANDLES;
		
		arglist params = {
			{ "granularity", toInterval(interval) },
			{ "count", std::to_string(max) }
		};

		json res = client->get(url, params);
		
		json arr = res["candles"];

		if(arr.size() == 0)
		{
			errorf("Failed to get candles @ %s", url);
			return candles;
		}
		else if(arr.size() < max)
		{
			warningf("Failed to get all requested candles: requested: %d, got: %d", max, arr.size());
		}
		successf("Received %d candles @ %dsec", arr.size(), interval);
		candles.resize(arr.size());
		
		unsigned int i = 0;
		for (json val : arr)
		{
			json mid = val["mid"];
			std::string open, high, low, close;
			double volume;
			volume = val["volume"].get<double>();
			open = mid["o"].get<std::string>();
			high = mid["h"].get<std::string>();
			low = mid["l"].get<std::string>();
			close = mid["c"].get<std::string>();
			candle c = { interval, std::stod(open), std::stod(high), std::stod(low), std::stod(close), volume };
			candles[i] = c;
			i++;
		}

		return candles;
	}
}

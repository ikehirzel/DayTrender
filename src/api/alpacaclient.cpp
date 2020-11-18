#include "alpacaclient.h"

#include <hirzel/fountain.h>

#include "../data/interval.h"

/*
curl -X GET -H "APCA-API-KEY-ID: {YOUR_API_KEY_ID}" -H "APCA-API-SECRET-KEY: {YOUR_API_SECRET_KEY}" https://{apiserver_domain}/v2/account
*/

namespace daytrender
{
	AlpacaClient::AlpacaClient(const std::vector<std::string>& credentials) : TradeClient(credentials)
	{	
		trade = new RestClient(ALPACA_BASE_URL, {
			{"APCA-API-KEY-ID", credentials[0] },
			{ "APCA-API-SECRET-KEY", credentials[1] },
			{ "Content-Type", "application/json" }
		});

		data = new RestClient(ALPACA_DATA_URL, {
			{"APCA-API-KEY-ID", credentials[0] },
			{ "APCA-API-SECRET-KEY", credentials[1] },
			{ "Content-Type", "application/json" }
		});
	}

	AlpacaClient::~AlpacaClient()
	{
		delete data;
		delete trade;
	}

	std::string AlpacaClient::server_time()
	{
		json time = trade->get(ALPACA_CLOCK);
		return time[ALPACA_TIMESTAMP].get<std::string>();
	}

	candleset AlpacaClient::getCandles(const std::string& ticker, unsigned int interval, unsigned int max)
	{
		max = (max > 0) ? max : ALPACA_MAX_CANDLES;
		candleset candles;
		std::string url = ALPACA_BARS "/" + toInterval(interval);

		arglist params = {
			{ "symbols", ticker },
			{ "limit", std::to_string(max) }
		};
				
		json res = data->get(url, params);

		json arr = res[ticker];

		if(arr.size() == 0)
		{
			errorf("Failed to get candles @ %s", url);
			return candles;
		}

		else if(arr.size() < max)
		{
			warningf("Failed to get all requested candles: requested: %d, got: %d", max, arr.size());
		}

		candles.resize(arr.size());

		unsigned int i = 0;
		for (json val : arr)
		{
			double open, high, low, close, volume;
			open = val["o"].get<double>();
			high = val["h"].get<double>();
			low = val["l"].get<double>();
			close = val["c"].get<double>();
			volume = val["v"].get<double>();
			candle c = { open, high, low, close, volume };
			candles[i] = c;
			i++;
		}
		successf("Received %d candles @ %dsec interval", candles.size(), interval);
		return candles;
	}

	bool AlpacaClient::market_open()
	{
		json clock = trade->get(ALPACA_CLOCK);
		return clock[ALPACA_IS_OPEN].get<bool>();
	}
	
	std::string AlpacaClient::toInterval(unsigned int interval)
	{
		switch(interval)
		{
			case MIN1:
				return ALPACA_1_MIN;
			case MIN5:
				return ALPACA_5_MIN;
			case MIN15:
				return ALPACA_15_MIN;
			case DAY:
				return ALPACA_1_DAY;
			default:
				return ALPACA_1_MIN;
		}
	}
}
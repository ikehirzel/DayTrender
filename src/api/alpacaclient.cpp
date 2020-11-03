#include "alpacaclient.h"

#include <iostream>
#include "../data/interval.h"

namespace daytrender
{
	AlpacaClient::AlpacaClient(const std::vector<std::string>& credentials) : TradeClient(credentials)
	{	
		trade = new RestClient(ALPACA_BASE_URL, {
			{"APCA-API-KEY-ID", this->credentials[0]},
			{ "APCA-API-SECRET-KEY", this->credentials[1] },
			{ "Content-Type", "application/json" }
		});

		data = new RestClient(ALPACA_DATA_URL);
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
		candleset candles;

		arglist params = {
			{ "symbols", ticker },
			{ "limit", std::to_string(max) }
		};
		
		std::string url = ALPACA_BARS + std::to_string(interval);
		
		json res = data->get(url, params);

		json arr = res[ticker];

		for (json val : arr)
		{
			double open, high, low, close, volume;
			open = val["o"].get<double>();
			high = val["h"].get<double>();
			low = val["l"].get<double>();
			close = val["c"].get<double>();
			volume = val["v"].get<double>();
			candle c = { interval, open, high, low, close, volume };
			candles.push_back(c);
		}

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
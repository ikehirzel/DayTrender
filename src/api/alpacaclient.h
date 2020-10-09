#pragma once

#include "tradeclient.h"

#include <string>

/*
	curl -X GET \
	-H "APCA-API-KEY-ID: {YOUR_API_KEYD}" \
	-H "APCA-API-SECRET-KEY: {YOUR_API_SECRET_KEY}"\
	https://{apiserver_domain}/v2/account

	using api.alpaca.markets or paper-api.alpaca.markets
*/

//link fragments
#define ALPACA_CLOCK		"/v2/clock"
#define ALPACA_BARS			"/v1/bars"
#define ALPACA_BASE_URL		"api.alpaca.markets"
#define ALPACA_DATA_URL		"data.alpaca.markets"

//JSON snippets
#define ALPACA_TIMESTAMP	"timestamp"
#define ALPACA_IS_OPEN		"is_open"

//Intervals
#define ALPACA_1_MIN		"1Min"
#define ALPACA_5_MIN		"5Min"
#define ALPACA_15_MIN		"15Min"
#define ALPACA_1_DAY		"1D"

namespace daytrender
{
	#define ALPACA_MAX_CANDLES 1000
	
	class AlpacaClient : public TradeClient
	{
	private:
		RestClient* data, *trade;
	public:
		AlpacaClient(const std::vector<std::string>& credentials);
		~AlpacaClient();

		std::string toInterval(unsigned int interval);
		
		candleset getCandles(const std::string& ticker, unsigned int interval,
			unsigned int max = ALPACA_MAX_CANDLES);
			
		std::string server_time();
		bool market_open();
	};
}
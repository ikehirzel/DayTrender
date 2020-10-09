#pragma once

#include "tradeclient.h"

namespace daytrender
{
	//constants for the different candle granularities
	#define OANDA_5_SEC		"S5"
	#define OANDA_10_SEC	"S10"
	#define OANDA_15_SEC	"S15"
	#define OANDA_30_SEC	"S30"
	#define OANDA_1_MIN		"M1"
	#define OANDA_2_MIN		"M2"
	#define OANDA_4_MIN		"M4"
	#define OANDA_5_MIN		"M5"
	#define OANDA_10_MIN	"M10"
	#define OANDA_15_MIN	"M15"
	#define OANDA_30_MIN	"M30"
	#define OANDA_1_HOUR	"H1"
	#define OANDA_2_HOUR	"H2"
	#define OANDA_3_HOUR	"H3"
	#define OANDA_4_HOUR	"H4"
	#define OANDA_6_HOUR	"H6"
	#define OANDA_8_HOUR	"H8"
	#define OANDA_12_HOUR	"H12"
	#define OANDA_1_DAY		"D"
	#define OANDA_1_WEEK	"W"
	#define OANDA_1_MONTH	"M"
	
	//constants for the different currency pairs
	#define OANDA_EUR_USD	"EUR_USD"
	
	//constants for the url fragments
	#define OANDA_BASE_URL		"api-fxpractice.oanda.com"
	#define OANDA_ACCOUNTS		"/v3/accounts"
	#define OANDA_INSTRUMENTS	"/instruments"
	#define OANDA_CANDLES		"/candles"
	#define OANDA_PRICE			""
	#define OANDA_CONFIG		"/configuration"
	#define OANDA_ORDERS		"/orders"

	#define OANDA_NO_MARGIN		"1.00"
	#define OANDA_MAX_CANDLES	5000
	#define OANDA_BID_CANDLES	"B"
	#define OANDA_ASK_CANDLES	"A"
	#define OANDA_MID_CANDLES	"M"

	class OandaClient : public TradeClient
	{
	private:
		RestClient* client;
		std::string username, accountid, token;
	public:
		OandaClient(const std::vector<std::string>& credentials);
		~OandaClient();
		double getBalance();
		std::string getAccounts();
		bool setMargin(double marginRate);
		bool marketOrder(const std::string& pair, double units);
		std::string getPositions();
		std::string toInterval(unsigned int interval);
		candleset getCandles(const std::string& ticker, unsigned int interval,
			unsigned int max = OANDA_MAX_CANDLES);
	};
}
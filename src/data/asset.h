#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "candle.h"
#include "action.h"
#include "paperaccount.h"
#include "tradealgorithm.h"

#ifndef PAPER_BY_DEFAULT
	#define PAPER_TRADING false
#else
	#define PAPER_TRADING true
#endif

// algorithm constants
#define MAX_ALGORITHM_WINDOW	250U
#define MIN_ALGORITHM_WINDOW	8U

// interval constants
	
// PaperAccount constants
#define PAPER_ACCOUNT_INITIAL	500U
	
#define FOREX_FEE				0.00007
#define FOREX_MINIMUM			0.01
#define FOREX_INITIALS			{ FOREX_FEE, FOREX_MINIMUM }

#define STOCK_FEE				0.0
#define STOCK_MINIMUM			1.0
#define STOCK_INITIALS			{ STOCK_FEE, STOCK_MINIMUM }

#define CRYPTO_FEE				0.0025
#define CRYPTO_MINIMUM			0.001
#define CRYPTO_INITIALS			{ CRYPTO_FEE, CRYPTO_MINIMUM }

#define PAPER_INITIALS			{ FOREX_INITIALS }

// Asset constants

#define FOREX_INDEX			0U
#define FOREX_LABEL			"Forex"

#define STOCK_INDEX			1U
#define STOCK_LABEL			"Stocks"

#define CRYPTO_INDEX		2U
#define CRYPTO_LABEL		"Crypto"

//temporarily set to just forex for testing
#define ASSET_TYPE_COUNT	2

#define ASSET_LABELS		{ FOREX_LABEL, STOCK_LABEL }

#define FOREX_INTERVALS		{ MIN5, MIN15, HOUR1 }
#define STOCK_INTERVALS		{ MIN5, MIN15, HOUR1 }
#define CRYPTO_INTERVALS	{ MIN5, MIN15, HOUR1 }
#define BACKTEST_INTERVALS	{ FOREX_INTERVALS, STOCK_INTERVALS }

namespace daytrender
{
	class TradeClient;
	class OandaClient;
	class AlpacaClient;
	
	typedef std::pair<candleset, algorithm_data> asset_data;

	extern const char* asset_labels[];
	extern double paper_initials[ASSET_TYPE_COUNT][2];
	extern unsigned int backtest_intervals[ASSET_TYPE_COUNT][3];

	class Asset
	{
		//function pointer to things like nothing, sell, buy, etc...
	protected:
		bool paper = PAPER_TRADING, live = false;
		unsigned tick = 0, interval = 0, window = 0, type = 0;
		double maxRisk = 0.9;
		std::string ticker;
		asset_data data;
		
		TradeClient* client;
		TradeAlgorithm* algo;
		//basePaperAccount will be a pointer holding a default copy of an unused paper account
		//papaerAccount is the current paperAccount
		PaperAccount paperAccount;
		
	public:

		Asset(unsigned int assetIndex, TradeClient *client, const std::string &ticker, TradeAlgorithm* algo,
			unsigned int interval, unsigned int window);

		void update();

		inline asset_data getData() const { return data; }
		inline TradeAlgorithm* getAlgorithm() const { return algo; }
		inline std::string getTicker() const { return ticker; }
		inline unsigned int getInterval() const { return interval; }
		inline unsigned getType() const { return type; }
		inline bool isLive() const { return live; }
		
		inline void setLive(bool _live) { live = _live; }
	};
}
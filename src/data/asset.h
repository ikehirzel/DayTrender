#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "candle.h"
#include "action.h"
#include "paperaccount.h"
#include "tradealgorithm.h"

// algorithm constants
#define MAX_ALGORITHM_WINDOW	50
#define MIN_ALGORITHM_WINDOW	4

// interval constants
	
// PaperAccount constants
#define PAPER_ACCOUNT_INITIAL	500
	
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

// The amount of asset types there are. Currently only forex and stocks
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

	extern const char* asset_labels[];
	extern double paper_initials[ASSET_TYPE_COUNT][2];
	extern unsigned int backtest_intervals[ASSET_TYPE_COUNT][3];

	class Asset
	{
	protected:
		bool paper = true, live = false;
		int tick = 0, interval = 0, candle_count = 0, type = 0;
		double maxRisk = 0.9;
		std::string ticker;
		algorithm_data data;
		
		TradeClient* client;
		TradeAlgorithm* algo;
		PaperAccount paperAccount;
		
	public:
		Asset(int assetIndex, TradeClient *client, const std::string &ticker, TradeAlgorithm* algo,
			int interval, const std::vector<int>& _ranges, bool _paper);

		void update();

		inline algorithm_data getData() const { return data; }
		inline TradeAlgorithm* getAlgorithm() const { return algo; }
		inline std::string getTicker() const { return ticker; }
		inline int getInterval() const { return interval; }
		inline int getType() const { return type; }
		inline bool isLive() const { return live; }
	};
}
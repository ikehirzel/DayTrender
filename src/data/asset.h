#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "paperaccount.h"
#include "tradealgorithm.h"

namespace daytrender
{
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

	#define PAPER_INITIALS			{ FOREX_INITIALS, STOCK_INITIALS, CRYPTO_INITIALS }

	// Asset constants

	#define FOREX_INDEX			0U
	#define FOREX_LABEL			"Forex"

	#define STOCK_INDEX			1U
	#define STOCK_LABEL			"Stocks"

	#define CRYPTO_INDEX		2U
	#define CRYPTO_LABEL		"Crypto"

	//temporarily set to just forex for testing
	#define ASSET_TYPE_COUNT	1
	#define ASSET_LABELS		{ FOREX_LABEL }

	#define FOREX_INTERVALS		{ MIN5, MIN15, HOUR1 }
	#define STOCK_INTERVALS		{ MIN5, MIN15, HOUR1 }
	#define CRYPTO_INTERVALS		{ MIN5, MIN15, HOUR1 }
	#define BACKTEST_INTERVALS	{ FOREX_INTERVALS, STOCK_INTERVALS, CRYPTO_INTERVALS }

	class TradeClient;
	class OandaClient;
	class PaperAccount;
	class AlpacaClient;
	
	typedef std::pair<candle_data, algorithm_data> asset_data;

	extern const char* asset_labels[];

	class Asset
	{
		//function pointer to things like nothing, sell, buy, etc...
		typedef void (Asset::*actionFunc)(PaperAccount*);
	protected:
		bool paper = PAPER_TRADING;
		unsigned int tick = 0, interval = 0, window = 0, index = 0;
		double maxRisk = 1.0;
		std::string ticker;
		std::vector<unsigned int> backtestIntervals;
		asset_data data;
		
		TradeClient* client;
		TradeAlgorithm* algo;
		//basePaperAccount will be a pointer holding a default copy of an unused paper account
		//papaerAccount is the current paperAccount
		PaperAccount basePaperAccount, paperAccount;
		
		actionFunc actions[3];
		// Callbacks for algorithm actions

		// Callback for ACTION_NOTHING
		void nothing(PaperAccount *account = nullptr) {}
		// Callback for ACTION_BUY
		void buy(PaperAccount* account = nullptr);
		// Callback for ACTION_SELL
		void sell(PaperAccount* account = nullptr);
		
	public:

		Asset(unsigned int assetIndex, TradeClient* client, TradeAlgorithm* algo,
			const std::string& ticker, unsigned int interval, unsigned int window);

		void update();
		
		PaperAccount backtest(TradeAlgorithm* algorithm, const candleset& candles,
			unsigned int inter, unsigned int win);
		
		//tests the current algorithm with current window and interval
		PaperAccount testCurrentConstraints();
		
		PaperAccount findBestWindow(TradeAlgorithm* algorithm, unsigned int inter);
		
		//tests the current algorithm 
		PaperAccount findBestWindow(TradeAlgorithm* algorithm, unsigned int inter,
			const candleset& c = {});
			
		//tests an algorithm againts several windows and intervals
		PaperAccount findBestConstraints(TradeAlgorithm* algorithm);
		
		inline asset_data getData() const { return data; }
		inline TradeAlgorithm* getAlgorithm() const { return algo; }
		inline std::string getTicker() const { return ticker; }
		inline unsigned int getInterval() const { return interval; }
	};
}
#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "candle.h"
#include "../api/action.h"
#include "paperaccount.h"
#include "../api/algorithm.h"
#include "../api/client.h"

// algorithm constants
#define MAX_ALGORITHM_WINDOW	50
#define MIN_ALGORITHM_WINDOW	4


#define PAPER_ACCOUNT_INITIAL 500.0

namespace daytrender
{
	struct asset_info
	{
		double shares = 0.0;
		double risk = 0.0;
		bool live = false;
		bool paper = true;
	};

	class Asset
	{
	protected:
		bool paper = true, live = false;
		int tick = 0, interval = 0, candle_count = 0, type = 0;
		double risk = 0.9;
		std::string ticker;
		algorithm_data data;
		std::vector<int> ranges;
		
		const Client* client;
		const Algorithm* algo;
		PaperAccount paperAccount;
		
	public:
		Asset(int _type, const Client* _client, const std::string &_ticker, const Algorithm* _algo,
			int _interval, double _risk, const std::vector<int>& _ranges, bool _paper);

		void update();

		asset_info getAssetInfo() const;
		inline algorithm_data getData() const { return data; }
		inline const Algorithm& getAlgorithm() const { return *algo; }
		inline const Client& getClient() const { return *client; }
		inline const std::string& getTicker() const { return ticker; }
		inline int getInterval() const { return interval; }
		inline int getType() const { return type; }
		inline bool isLive() const { return live; }
		inline int getRisk() const { return risk; }
	};
}
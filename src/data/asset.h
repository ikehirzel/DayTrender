#pragma once

#include <string>
#include <vector>

#include "candle.h"
#include "paperaccount.h"
#include "../api/algorithm.h"
#include "../api/client.h"
#include <hirzel/sysutil.h>

// algorithm constants
#define MAX_ALGORITHM_WINDOW	50
#define MIN_ALGORITHM_WINDOW	4


#define PAPER_ACCOUNT_INITIAL 500.0

namespace daytrender
{
	struct AssetInfo
	{
		double shares = 0.0;
		double risk = 0.0;
		bool live = false;
		bool paper = true;
	};

	class Asset
	{
	protected:
		bool paper = true;
		bool live = false;

		int interval = 0;
		int candle_count = 0;
		int type = 0;

		long long last_update = 0;

		double risk = 0.9;

		std::string ticker;
		AlgorithmData data;
		std::vector<int> ranges;
		
		const Client* client;
		const Algorithm* algo;
		PaperAccount paperAccount;
		
	public:
		Asset(int _type, const Client* _client, const std::string &_ticker, const Algorithm* _algo,
			int _interval, double _risk, const std::vector<int>& _ranges, bool _paper);

		void update();
		AssetInfo get_info() const;

		// inline getter functions
		inline AlgorithmData get_data() const { return data; }
		inline const Algorithm& get_algorithm() const { return *algo; }
		inline const Client& get_client() const { return *client; }
		inline const std::string& get_ticker() const { return ticker; }
		inline int get_interval() const { return interval; }
		inline int get_type() const { return type; }
		inline bool is_live() const { return live; }
		inline int get_risk() const { return risk; }
		inline bool should_update() const
		{
			if ((hirzel::sys::get_seconds() - last_update) > interval)
			{
				if (client->market_open())
				{
					return true;
				}
			}
			return false;
		}
	};
}
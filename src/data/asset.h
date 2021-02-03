#pragma once

#include <string>
#include <vector>

#include "candle.h"
#include "../api/paperaccount.h"
#include "../api/algorithm.h"
#include "../api/client.h"

// algorithm constants
#define MAX_ALGORITHM_WINDOW	100
#define MIN_ALGORITHM_WINDOW	4


#define PAPER_ACCOUNT_INITIAL 500.0

namespace daytrender
{
	class Asset
	{
	private:
		bool _paper = true;
		bool _live = false;

		int _interval = 0;
		int _candle_count = 0;
		int _type = 0;

		long long _last_update = 0;

		double _risk = 0.0;

		std::string _ticker;
		AlgorithmData _data;
		std::vector<int> _ranges;
		
		Client* _client;
		const Algorithm* _algo;
		typedef bool (Asset::*AssetAction)();
		AssetAction _actions[Action::COUNT];
		
		// client wrappers
		inline CandleSet get_candles() const
		{
			return _client->get_candles(_ticker, _interval, _candle_count, _algo->data_length());
		}

		bool enter_long();
		bool exit_long();
		bool enter_short();
		bool exit_short();

	public:
		Asset(int type, Client* client, const std::string &ticker, const Algorithm* algo,
			int interval, double risk, const std::vector<int>& ranges, bool paper);

		void update();
		bool should_update() const;


		// inline getter functions
		inline AlgorithmData data() const { return _data; }
		inline const Algorithm* algorithm() const { return _algo; }
		inline Client* client() const { return _client; }
		inline const std::string& ticker() const { return _ticker; }
		inline int interval() const { return _interval; }
		inline int type() const { return _type; }
		inline bool is_live() const { return _live && _client->is_live(); }
		inline double risk() const { return _risk; }
		inline bool is_paper() const { return _paper; }
		inline int data_length() const { return _algo->data_length(); }
	};
}
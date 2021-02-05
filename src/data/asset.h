#pragma once

#include <string>
#include <vector>

#include "candle.h"
#include "../api/algorithm.h"
#include "../api/client.h"

namespace daytrender
{
	class Asset
	{
	private:
		bool _shorting_enabled = false;
		bool _live = false;
		
		int _interval = 0;
		int _candle_count = 0;
		int _type = 0;
		int _closeout_buffer = 15; // time to close out position 

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

		inline double get_price() const 
		{
			return _client->get_price(_ticker);
		}

		inline double get_shares() const
		{
			return _client->get_shares(_ticker);
		}

		inline bool close_position()
		{
			return _client->market_order(_ticker, _client->get_shares(_ticker));
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
		inline bool shorting_enabled() const { return _shorting_enabled; }
		inline int data_length() const { return _algo->data_length(); }
	};
}
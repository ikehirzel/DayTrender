#pragma once

#include <string>
#include <vector>

#include "../data/candle.h"
#include "strategy.h"
#include "client.h"

namespace daytrender
{
	class Asset
	{
	private:
		mutable bool _live = false;
		int _candle_count = 0;
		int _interval = 0;
		int _type = -1;
		long long _last_update = 0;
		double _risk = 0.0;

		std::string _ticker;
		StrategyData _data;
		std::vector<int> _ranges;
		
		Client* _client;
		const Strategy* _strategy;
		typedef bool (Asset::*AssetAction)();
		AssetAction _actions[Action::COUNT];
		
		// client wrappers
		inline CandleSet get_candles() const
		{
			return _client->get_candles(_ticker, _interval, _candle_count, _strategy->data_length());
		}

	public:
		Asset(Client* client, const Strategy* strat, const std::string& ticker, int type,
			int interval, const std::vector<int>& ranges);

		void update();

		// inline getter functions
		inline const StrategyData& data() const { return _data; }
		inline const Strategy* strategy() const { return _strategy; }
		inline const Client* client() const { return _client; }
		inline const std::string& ticker() const { return _ticker; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline int interval() const { return _interval; }
		inline int type() const { return _type; }
		inline bool is_live() const { return _live && _client->is_live(); }
		inline double risk() const { return _risk; }
		inline int data_length() const { return _strategy->data_length(); }
	};
}
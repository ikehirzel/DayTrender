#pragma once

#include <string>
#include <vector>

#include "../data/candle.h"
#include "strategy.h"
#include "client.h"

#include <hirzel/data.h>

namespace daytrender
{
	class Asset
	{
	private:
		bool _bound = false;
		mutable bool _live = false;
		int _candle_count = 0;
		int _interval = 0;
		long long _last_update = 0;
		double _risk = 0.0;

		std::string _ticker;
		StrategyData _data;
		std::vector<int> _ranges;

		Strategy _strategy;
		
		typedef bool (Asset::*AssetAction)();
		AssetAction _actions[Action::COUNT];
		
		// client wrappers
		inline CandleSet get_candles(const Client& client) const
		{
			return client.get_candles(_ticker, _interval, _candle_count, _strategy.data_length());
		}

	public:
		Asset(const hirzel::Data& config, const std::string& dir);
		Asset() = default;

		void update(Client& client);

		// inline getter functions
		inline const StrategyData& data() const { return _data; }
		inline const Strategy& strategy() const { return _strategy; }
		inline const std::string& ticker() const { return _ticker; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline int interval() const { return _interval; }
		inline bool is_live() const { return _live; }
		inline double risk() const { return _risk; }
		inline int data_length() const { return _strategy.data_length(); }
	};
}
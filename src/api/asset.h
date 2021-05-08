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
		unsigned _candle_count = 0;
		unsigned _interval = 0;
		long long _last_update = 0;
		double _risk = 0.0;

		std::string _ticker;
		StrategyData _data;
		std::vector<int> _ranges;

		Strategy _strategy;
		
		// client wrappers
		inline Result<CandleSet> get_candles(const Client& client) const
		{
			return client.get_candles(_ticker, _interval, _candle_count);
		}

	public:
		Asset(const hirzel::Data& config, const std::string& dir);
		Asset() = default;

		unsigned update(Client& client);

		// inline getter functions
		inline const StrategyData& data() const { return _data; }
		inline const Strategy& strategy() const { return _strategy; }
		inline const std::string& ticker() const { return _ticker; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline unsigned interval() const { return _interval; }
		inline double risk() const { return _risk; }
		inline unsigned data_length() const { return _strategy.data_length(); }

	};
}
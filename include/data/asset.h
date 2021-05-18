#ifndef DAYTRENDER_ASSET_H
#define DAYTRENDER_ASSET_H

// local includes
#include <api/client.h>
#include <api/strategy.h>
#include <data/candle.h>

// standard libarary
#include <string>
#include <vector>

// external libraries
#include <hirzel/data.h>
#include <hirzel/util/sys.h>

namespace daytrender
{
	class Asset
	{
	private:
		// update timer
		unsigned _interval = 0;
		long long _last_update = 0;

		unsigned _candle_count = 0;
		double _risk = 0.0;

		std::string _ticker;
		Chart _data;
		std::vector<int> _ranges;

		Strategy _strategy;
		
		// client wrappers
		inline Result<PriceHistory> get_candles(const Client& client) const
		{
			return client.get_price_history(_ticker, _interval, _candle_count);
		}

	public:
		Asset(const hirzel::Data& config, const std::string& dir);
		Asset() = default;

		unsigned update(Client& client);
		inline bool should_update() const
		{
			return hirzel::sys::epoch_seconds() - _last_update >= _interval;
		}

		// inline getter functions
		inline const Chart& data() const { return _data; }
		inline const Strategy& strategy() const { return _strategy; }
		inline const std::string& ticker() const { return _ticker; }
		inline const std::vector<int>& ranges() const { return _ranges; }
		inline unsigned interval() const { return _interval; }
		inline double risk() const { return _risk; }
		inline unsigned data_length() const { return _strategy.data_length(); }
		inline bool is_bound() const { return _strategy.is_bound(); }
	};
}

#endif

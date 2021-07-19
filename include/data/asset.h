#ifndef DAYTRENDER_ASSET_H
#define DAYTRENDER_ASSET_H

// local includes
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
	private: // data members

		long long _last_update = 0;
		unsigned _interval = 0;
		std::vector<unsigned> _ranges;
		unsigned _candle_count = 0;
		std::string _ticker;
		Strategy _strategy;
		double _risk = 0.0;
		
	private: // initializer getters

		unsigned get_interval(const hirzel::Data& config) const;
		std::vector<unsigned> get_ranges(const hirzel::Data& config) const;
		Strategy get_strategy(const hirzel::Data& config, const std::string& dir) const;
		std::string get_ticker(const hirzel::Data& config) const;
		unsigned get_candle_count() const;
		double get_risk() const;

	
	public: // public functions

		Asset(const hirzel::Data& config, const std::string& dir);

		unsigned update(const PriceHistory& hist);
		inline bool should_update() const
		{
			return hirzel::sys::epoch_seconds() - _last_update >= _interval;
		}

		// inline getter functions
		inline const Strategy& strategy() const { return _strategy; }
		inline const std::string& ticker() const { return _ticker; }
		inline const std::vector<unsigned>& ranges() const { return _ranges; }
		inline unsigned interval() const { return _interval; }
		inline unsigned candle_count() const { return _candle_count; }
		inline double risk() const { return _risk; }
		inline unsigned data_length() const { return _strategy.data_length(); }
		inline bool is_bound() const { return _strategy.is_bound(); }
	};
}

#endif

#include <data/strategydata.h>

namespace daytrender
{
	StrategyData::StrategyData(const std::vector<int>& ranges,
		const CandleSet& candles, unsigned window)
	{
		_ranges = ranges;

		_candles = candles;
		_capacity = ranges.size();
		_dataset = new Indicator[_capacity];

		// initializing all the indicators to same size
		for (int i = 0; i < _capacity; i++)
		{
			_dataset[i] = { window };
		}
	}

	StrategyData::StrategyData(const StrategyData& other)
	{
		*this = other;
	}

	StrategyData::StrategyData(StrategyData&& other)
	{
		_dataset = other._dataset;
		_capacity = other._capacity;
		_size = other._size;
		_action = other._action;
		_label = other._label;
		_error = other._error;
		_ranges = other._ranges;
		_candles = other._candles;

		other._dataset = nullptr;
	}

	StrategyData::~StrategyData()
	{
		delete[] _dataset;
	}

	StrategyData& StrategyData::operator=(const StrategyData& other)
	{
		_ranges = other.ranges();
		_candles = other.candles();

		_size = other.size();
		_dataset = new Indicator[_size];

		for (int i = 0; i < _size; i++)
		{
			_dataset[i] = other[i];
		}

		return *this;
	}
}
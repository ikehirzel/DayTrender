#include "algorithmdata.h"

namespace daytrender
{
	AlgorithmData::AlgorithmData(const std::vector<int>& ranges, const CandleSet& candles)
	{
		_ranges = ranges;

		_candles = candles;
		_capacity = ranges.size();
		_dataset = new Indicator[_capacity];

		for (int i = 0; i < _capacity; i++)
		{
			_dataset[i] = { candles.end() };
		}
	}

	AlgorithmData::AlgorithmData(const AlgorithmData& other)
	{
		*this = other;
	}

	AlgorithmData::AlgorithmData(AlgorithmData&& other)
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

	AlgorithmData::~AlgorithmData()
	{
		delete[] _dataset;
	}

	AlgorithmData& AlgorithmData::operator=(const AlgorithmData& other)
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
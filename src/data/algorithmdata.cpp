#include "algorithmdata.h"

namespace daytrender
{
	AlgorithmData::AlgorithmData(const std::vector<int>& ranges, const CandleSet& candles)
	{
		_ranges = ranges;
		_candles = candles;

		_size = ranges.size() - 1;
		_dataset = new Indicator[_size];

		for (int i = 0; i < _size; i++)
		{
			_dataset[i] = { ranges[0] };
		}
	}

	AlgorithmData::AlgorithmData(const AlgorithmData& other)
	{
		*this = other;
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
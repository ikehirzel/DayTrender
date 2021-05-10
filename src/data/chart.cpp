#include <data/chart.h>


namespace daytrender
{
	Chart::Chart(const std::vector<int>& ranges,
		const PriceHistory& candles, unsigned data_length)
	{
		_ranges = ranges;

		_candles = candles;
		_size = ranges.size();
		_dataset = new Indicator[_size];

		// initializing all the indicators to same size
		for (int i = 0; i < _size; i++)
		{
			_dataset[i] = { data_length };
		}
	}

	Chart::Chart(const Chart& other)
	{
		*this = other;
	}

	Chart::Chart(Chart&& other)
	{
		_dataset = other._dataset;
		_size = other._size;
		_action = other._action;
		_label = other._label;
		_ranges = other._ranges;
		_candles = other._candles;

		other._dataset = nullptr;
	}

	Chart::~Chart()
	{
		delete[] _dataset;
	}

	Chart& Chart::operator=(const Chart& other)
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
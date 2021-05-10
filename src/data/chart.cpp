#include <data/chart.h>


namespace daytrender
{
	Chart::Chart(const std::vector<int>& ranges,
		const PriceHistory& candles, unsigned window)
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

	Chart::Chart(const Chart& other)
	{
		*this = other;
	}

	Chart::Chart(Chart&& other)
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
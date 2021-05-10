#include <data/pricehistory.h>

namespace daytrender
{
	PriceHistory::PriceHistory(unsigned size, unsigned interval)
	{
		_interval = interval;
		_size = size;
		_data = new Candle[_size];
	}

	PriceHistory::PriceHistory(PriceHistory&& other)
	{
		_data = other._data;
		_size = other._size;
		_interval = other._interval;
		
		_slice = other._slice;

		other._slice = true;
	}

	PriceHistory::PriceHistory(const PriceHistory& other)
	{
		*this = other;
	}

	PriceHistory::PriceHistory(Candle* parent_data, unsigned parent_size,
		unsigned parent_interval, unsigned offset, unsigned size)
	{
		_slice = true;
		_interval = parent_interval;
		_data = parent_data + offset;
		_size = size;
	}

	PriceHistory::~PriceHistory()
	{
		if (!_slice)
		{
			delete[] _data;
		}
	}

	PriceHistory& PriceHistory::operator=(const PriceHistory& other)
	{
		_interval = other.interval();
		_size = other.size();
		_data = new Candle[_size];

		for (int i = 0; i < _size; i++)
		{
			_data[i] = other.front(i);
		}
	
		return *this;
	}
}

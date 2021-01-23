#include "candle.h"

namespace daytrender
{
	CandleSet::CandleSet(int size, int interval)
	{
		_interval = interval;
		_size = size;
		_data = new Candle[_size];
	}

	CandleSet::CandleSet(const CandleSet& other)
	{
		*this = other;
	}

	CandleSet::CandleSet(const CandleSet& other, int offset, int size)
	{
		_slice = true;
		_size = size;
		_interval = other.interval();
		_data = other.data() + offset;
	}

	CandleSet::~CandleSet()
	{
		if (!_slice) delete[] _data;
	}

	CandleSet& CandleSet::operator=(const CandleSet& other)
	{
		_interval = other.interval();
		_size = other.size();
		_data = new Candle[_size];
		for (int i = 0; i < _size; i++)
		{
			_data[i] = other[i];
		}

		return *this;
	}
}
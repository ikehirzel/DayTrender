#include "candle.h"

#include <cstring>

namespace daytrender
{
	Candle::Candle(double open, double high, double low, double close, double volume)
	{
		_open = open;
		_high = high;
		_low = low;
		_close = close;
		_volume = volume;
	}

	std::string Candle::to_string() const
	{
		std::string out;
		out += "Candle:\n{";
		out += "\n\topen   : " + std::to_string(_open);
		out += "\n\thigh   : " + std::to_string(_high);
		out += "\n\tlow    : " + std::to_string(_low);
		out += "\n\tclose  : " + std::to_string(_close);
		out += "\n\tvolume : " + std::to_string(_volume);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const Candle& candle)
	{
		out << candle.to_string();
		return out;
	}

	CandleSet::CandleSet(unsigned size, unsigned end, int interval)
	{
		_interval = interval;
		_size = size;
		_end = end;
		_data = new Candle[_size];
	}

	CandleSet::CandleSet(CandleSet&& other)
	{
		_data = other._data;
		_size = other._size;
		_interval = other._interval;
		_end = other._end;
		_slice = other._slice;
		_error = other._error;

		other._slice = true;
	}

	CandleSet::CandleSet(const CandleSet& other)
	{
		*this = other;
	}

	CandleSet::CandleSet(Candle* parent_data, unsigned parent_size, int parent_interval, unsigned offset, unsigned size, unsigned end)
	{
		_slice = true;
		_interval = parent_interval;
		_data = parent_data;
		
		if (!_data)
		{
			_error = "slice data is nullptr";
		}
		else
		{
			_data += offset;
		}

		if (offset + size > parent_size)
		{
			_error = "slice reaches out of parent's bounds";
		}
		else
		{
			_size = size;

			if (end > _size)
			{
				_error = "end is greater than size";
			}
			else
			{
				_end = end;
			}
		}
	}

	CandleSet::~CandleSet()
	{
		if (!_slice)
		{
			delete[] _data;
		}
	}

	CandleSet& CandleSet::operator=(const CandleSet& other)
	{
		_interval = other.interval();
		_error = other.error();
		_size = other.size();
		_end = other.end();
		_data = new Candle[_size];

		for (int i = 0; i < _size; i++)
		{
			_data[i] = other.front(i);
		}
	
		return *this;
	}
}
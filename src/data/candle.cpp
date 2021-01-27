#include "candle.h"

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
		out += "\n\thigh   : " + std::to_string(_open);
		out += "\n\tlow    : " + std::to_string(_open);
		out += "\n\tclose  : " + std::to_string(_open);
		out += "\n\tvolume : " + std::to_string(_open);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const Candle& candle)
	{
		out << candle.to_string();
		return out;
	}

	CandleSet::CandleSet(int size, int interval)
	{
		_interval = interval;
		_size = size;
		_data = new Candle[_size];
	}

	CandleSet::CandleSet(const CandleSet& other) { *this = other; }

	CandleSet::CandleSet(const CandleSet& other, unsigned offset, unsigned size, unsigned shamt)
	{
		_slice = true;
		_size = size;
		_shamt = shamt;
		_interval = other.interval();
		_data = other.data() + offset;
		if (size > other.size())
		{
			_error = "CandleSet slice is larger than base CandleSet";
		}
		else if (size == 0)
		{
			_error = "CandleSet size is 0";
		}
		else if (shamt > size)
		{
			_error = "Shift amount is larger than size";
		}
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
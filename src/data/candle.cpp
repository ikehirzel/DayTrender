#include <data/candle.h>


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
}

#include "candle.h"

#include <cmath>

namespace daytrender
{
	candle::candle(const std::string& time, double open, double high, double low, double close, unsigned int volume)
	{
		this->time = time;
		this->open = open;
		this->high = high;
		this->low = low;
		this->close = close;
		this->volume = volume;
		
		change = close - open;
		volatility = high - low / (abs(open - close) + 1);
		
		if(change > 0)
		{
			movement = BULLISH;
		}
		else if(change == 0)
		{
			movement = SIDEWAYS;
		}
		else if(change < 0)
		{
			movement = BEARISH;
		}
	}
	
	std::ostream& operator<<(std::ostream& out, const candle& c)
	{
		out << "candle:\n{\n\tTime   :\t" << c.time << "\n\tOpen   :\t" << c.open << "\n\tHigh   :\t" << c.high << "\n\tLow    :\t" << c.low << "\n\tClose  :\t" << c.close << "\n\tVolume :\t" << c.volume << "\n}";
		return out;
	}
}
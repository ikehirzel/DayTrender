#pragma once

#include <iostream>
#include <vector>

namespace daytrender
{
	enum Movement
	{
		BEARISH = -1, SIDEWAYS = 0, BULLISH = 1
	};
	
	struct candle
	{
		std::string time;
		unsigned int volume = 0;
		double open = 0.0, high = 0.0, low = 0.0, close = 0.0,
			volatility = 0.0, change = 0.0;
			
		int movement = SIDEWAYS;
		
		candle() = default;
		candle(const std::string& time, double open, double high,
			double low, double close, unsigned int volume);

		friend std::ostream& operator<<(std::ostream& out, const candle& c);
	};

	typedef std::vector<candle> candleset;
	typedef std::pair<unsigned int, candleset> candle_data;
}

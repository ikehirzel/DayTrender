#pragma once

#include <iostream>
#include <vector>

namespace daytrender
{
	typedef std::vector<double> candle;
	typedef std::vector<candle> candleset;
	typedef std::pair<unsigned int, candleset> candle_data;

	void printCandle(const candle& c);
	void calculateCandle(candle& c);
}

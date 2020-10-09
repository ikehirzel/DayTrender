#pragma once

#include "indicator.h"

namespace daytrender
{
	class AvgTrendLine : public Indicator
	{
	public:
		AvgTrendLine(unsigned int denom);
		indicator_data calculate(const candleset& candles, unsigned int index, unsigned int window);
	};	
}
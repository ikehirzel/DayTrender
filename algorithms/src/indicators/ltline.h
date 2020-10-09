#pragma once

#include "indicator.h"

namespace daytrender
{	
	class LowerTrendLine : public Indicator
	{
	public:
		LowerTrendLine(unsigned int denom);
		indicator_data calculate(const candleset& candles, unsigned int index, unsigned int window);
	};
}
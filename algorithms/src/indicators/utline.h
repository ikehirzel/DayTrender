#pragma once

#include "indicator.h"

namespace daytrender
{
	class UpperTrendLine : public Indicator
	{
	public:
		UpperTrendLine(unsigned int denom);
		indicator_data calculate(const candleset& candles, unsigned int index, unsigned int window);
	};
}
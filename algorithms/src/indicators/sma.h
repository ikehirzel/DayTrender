#pragma once

#include "indicator.h"

namespace daytrender
{
	class SMA: public Indicator
	{
	public:
		SMA(unsigned int denom);
		indicator_data calculate(const candleset& candles, unsigned int index, unsigned int window);
	};
}
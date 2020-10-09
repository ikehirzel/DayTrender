#pragma once

#include "tradealgorithm.h"

namespace daytrender
{
	class UpperTrendLine;
	class LowerTrendLine;
	class AvgTrendLine;
	
	class SimpleSR: public TradeAlgorithm
	{
	private:
		UpperTrendLine* upperTrend;
		LowerTrendLine* lowerTrend;
	public:
		SimpleSR();
		int process(const indicator_dataset& dataset, const candleset& candles, unsigned int index);
	};
}
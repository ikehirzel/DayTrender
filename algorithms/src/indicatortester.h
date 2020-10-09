#pragma once 

#include "tradealgorithm.h"

namespace daytrender
{
	class AvgTrendLine;
	class UpperTrendLine;
	class LowerTrendLine;
	class EMA;
	class SMA;
	
	class IndicatorTester : public TradeAlgorithm
	{
	private:
		AvgTrendLine* atl;
		UpperTrendLine* utl;
		LowerTrendLine* ltl;
		EMA* ema;
		SMA* sma;
	public:
		IndicatorTester();
		int process(const indicator_dataset& dataset, const candleset& candles,
			unsigned int index);
	};
}
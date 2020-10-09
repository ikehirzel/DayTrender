#include "indicatortester.h"

#include "../data/indicators/atline.h"
#include "../data/indicators/utline.h"
#include "../data/indicators/ltline.h"
#include "../data/indicators/ema.h"
#include "../data/indicators/sma.h"

#define INDICATOR_TESTER_NAME "Indicator Tester"

namespace daytrender
{
	IndicatorTester::IndicatorTester() :
		TradeAlgorithm(INDICATOR_TESTER_NAME)
	{
		atl = new AvgTrendLine(1);
		utl = new UpperTrendLine(1);
		ltl = new LowerTrendLine(1);
		
		ema = new EMA(1);
		sma = new SMA(8);
		
		indicators = { atl, utl, ltl, ema, sma };
	}
	
	int IndicatorTester::process(const indicator_dataset& dataset, const candleset& candles,
		unsigned int index)
	{
		std::cout << "Indicator Tester: Process " << candles.size() << " candles starting at " << index << std::endl;
		return 0;
	}
}
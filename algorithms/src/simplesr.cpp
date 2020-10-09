#include "simplesr.h"

#include "../data/indicators/utline.h"
#include "../data/indicators/ltline.h"
#include "../data/action.h"

#define SIMPLE_SR_NAME "Simple SR"

namespace daytrender
{
	SimpleSR::SimpleSR() : TradeAlgorithm(SIMPLE_SR_NAME) 
	{
		upperTrend = new UpperTrendLine(1);
		lowerTrend = new LowerTrendLine(1);
		
		indicators = { lowerTrend, upperTrend };
	}
	
	int SimpleSR::process(const indicator_dataset& dataset, const candleset& candles,
		unsigned int index)
	{
		std::vector<double> ltl = dataset.at(lowerTrend->getName());
		std::vector<double> utl = dataset.at(upperTrend->getName());
		
		candle top = candles[index], last = candles[index - 1];
		
		if (top.close < utl[index] && top.open > utl[index])
		{
			return ACTION_SELL;
		}
		else if (top.close > ltl[index] && top.open < ltl[index - 1])
		{
			return ACTION_BUY;
		}
		
		return ACTION_NOTHING;
	}
}
#define INDICATORS 2
#define DATA_LENGTH 5

#include <candle.h>
#include <strategydefs.h>
#include <iostream>

void EMA(Indicator& data, const CandleSet& candles, int range)
{
	double multiplier = 2.0 / (double)(range + 1);

	double sum = 0.0;
	for(int i = -range + 1; i <= 0; i++)
	{
		sum += candles[i].close();
	}
	data[0] = sum / (double)range;

	for (int i = 1; i < data.size(); i++)
	{
		data[i] = candles[i].close() * multiplier + data[i - 1] * (1.0 - multiplier);
	}
}


void strategy(StrategyData& out)
{
	init_strategy(out, "Simple MA");
	const Indicator& longma = add_indicator(out, EMA, "long");
	const Indicator& shortma = add_indicator(out, EMA, "short");

	if (shortma.back() > longma.back() && shortma.back(1) < longma.back(1))
	{
		out.enter_long();
		//out.exit_short();
	}
	else if (shortma.back() < longma.back() && shortma.back(1) > longma.back(1))
	{
		//out.enter_short();
		out.exit_long();
	}
}

#define INDICATORS 2

#include <candle.h>
#include <algodefs.h>
#include <iostream>

void EMA(Indicator& data, const CandleSet& candles, int range)
{
	double multiplier = 2.0 / (double)(range + 1);
	// candles.size() - ranges[0] should be the shamt
	int start_index = candles.size() - data.size(); // this is correct

	double sum = 0.0;
	for(int i = start_index - range + 1; i <= start_index; i++)
	{
		sum += candles[i].c();
	}
	data[0] = sum / (double)range;

	int ci = start_index + 1;
	for (int i = 1; i < data.size(); i++)
	{
		data[i] = candles[ci++].c() * multiplier + data[i - 1] * (1.0 - multiplier);
	}
}

void algorithm (AlgorithmData& out)
{
	init_algorithm(out, "Simple MA");

	const Indicator& longma = add_indicator(out, EMA, "long");
	const Indicator& shortma = add_indicator(out, EMA, "short");

	if (shortma.back() > longma.back() && shortma.back(1) < longma.back(1))
	{
		out.buy();
	}
	else if (shortma.back() < longma.back() && shortma.back(1) > longma.back(1))
	{
		out.sell();
	}
}

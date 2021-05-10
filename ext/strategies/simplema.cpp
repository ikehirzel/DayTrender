#define DATA_LENGTH 5
#define LABEL "Simple MA"

#include <api/strategy_api.h>

void EMA(Indicator& data, const PriceHistory& candles, unsigned range)
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

std::vector<IndicatorConfig> config = 
{
	{ EMA, "EMA", "long" },
	{ EMA, "EMA", "short" }
};

Action strategy(const Chart& chart)
{
	const Indicator& longma = chart[0];
	const Indicator& shortma = chart[1];

	if (shortma.back() > longma.back() && shortma.back(1) < longma.back(1))
	{
		return ENTER_LONG;
	}
	else if (shortma.back() < longma.back() && shortma.back(1) > longma.back(1))
	{
		return EXIT_LONG;
	}

	return NOTHING;
}

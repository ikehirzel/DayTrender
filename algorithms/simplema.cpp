#include <candle.h>
#include <algodefs.h>
#include <iostream>

indicator EMA(const candleset& candles, int range)
{
	indicator data(candles.size);
	
	double multiplier = 2.0 / (double)(range + 1);

	double sum = 0.0;
	int initLength = 5;

	for(int i = 0; i < initLength; i++)
	{
		sum += candles[i].close;
	}

	data[0] = sum / (double)initLength;

	for (int i = 1; i < candles.size; i++)
	{
		data[i] = candles[i].close * multiplier + data[i - 1] * (1.0 - multiplier);
	}

	return data;
}

int arg_count() { return 3; }

bool algorithm (algorithm_data& out)
{
	// initializing algorithm data
	if (!init_algorithm(out, "Simple MA")) return false;

	const indicator& longma = add_indicator(out, 0, EMA, "EMA", "long");
	const indicator& shortma = add_indicator(out, 1, EMA, "EMA", "short");
	// processing algorithm
	//std::cout << "Longma: " << longma.size() << std::endl;

	if (shortma.back() > longma.back() && shortma[shortma.size() - 2] < longma[longma.size() - 2])
	{
		out.buy();
	}
	else if (shortma.back() < longma.back() && shortma[shortma.size() - 2] > longma[longma.size() - 2])
	{
		out.sell();
	}

	return true;
}

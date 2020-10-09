#include "../src/data/action.h"
#include "indicators/ema.h"
#include "../src/data/candle.h"

typedef std::map<std::string, std::vector<double>> indicator_dataset;
typedef std::pair<indicator_dataset, int> algorithm_data;

extern "C"
{
	unsigned int window = 130;
	EMA ema(4);
algorithm_data process(const daytrender::candleset& candles, unsigned int index)
{


	algorithm_data out;
	indicator_dataset dataset;
	indicator_data emaData = ema.calculate(candles, index, window);
	dataset.insert(emaData);

	std::vector<double> emavals = emaData.second;
	double price, lastPrice, ma, lastMa;

	daytrender::candle top = candles[index];


	price = top.close;
	lastPrice = candles[index - 1].close;
	ma = emavals.back();
	lastMa = emavals[emavals.size() - 1];
	//bullish crossover
	if(price > ma && lastPrice < lastMa)
	{
		out.second = ACTION_BUY;
	}
	//bearish crossover
	else if (price < ma && lastPrice > lastMa)
	{
		out.second = ACTION_SELL;
	}
	out.second = ACTION_NOTHING;
	return out;
}

std::string getName()
{
	return "Simple MA";
}
}

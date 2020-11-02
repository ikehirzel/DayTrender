
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>


namespace daytrender
{
	struct candle
	{
		double open = 0.0,
		high = 0.0,
		low = 0.0,
		close = 0.0,
		volume = 0.0,
		interval = 0.0,
		change = 0.0,
		volatility = 0.0;

		candle() = default;

		candle(const std::vector<double>& data)
		{
			open = data[0];
			high = data[1];
			low = data[2];
			close = data[3];
			volume = data[4];
			interval = data[5];
			change = data[6];
			volatility = data[7];
		}
	};
}

using namespace daytrender;

typedef std::pair<std::string, std::vector<double>> indicator_data;
typedef std::unordered_map<std::string, indicator_data> indicator_dataset;
typedef std::pair<indicator_dataset, unsigned int> algorithm_data;
typedef std::vector<candle> candleset;



struct EMA
{
	double ratio = 0.0;
	EMA(double _ratio)
	{
		ratio = _ratio;
	}

	std::string getName()
	{
		return "EMA";
	}

	indicator_data calculate(const candleset& candles, unsigned int index, unsigned int window)
	{
		unsigned int range = window * ratio;
		unsigned int startIndex = (index + 1) - window;
		std::vector<double> data(window);
		if(index >= candles.size() || startIndex >= candles.size())
		{
			std::cout << getName() << ": index out of bounds\n";
			return { getName(), data };
		}
		
		

		// Script:
		double multiplier = 2.0 / (double)(range + 1);
		double sum = 0.0;
		unsigned int initLength = 5;
		for(unsigned int i = 0; i < startIndex + initLength; i++)
		{
			sum += candles[i].close;
		}
		data[0] = sum / (double)initLength;
		unsigned int dataIndex = 1;
		for(unsigned int i = startIndex + 1; i <= index; i++)
		{
			double a = candles[i].close * multiplier;
			double b = data[dataIndex - 1] * (1.0 - multiplier);
			data[dataIndex] = a + b;
			dataIndex++;
		}
		
		return { getName(), data };
	}
};


EMA longma_INDICATOR(1.0);
EMA shortma_INDICATOR(0.25);

unsigned int getAction(const indicator_dataset& dataset, const candleset& candles, unsigned int index)
{
	
	std::vector<double> longma = dataset.at("longma").second;
	std::vector<double> shortma = dataset.at("shortma").second;
	
	double lastShort = shortma[index - 1];
	double lastLong = longma[index - 1];
	//bullish crossover
	if(shortma[index] > longma[index] && lastShort < lastLong)
	{
		return 2;
	}
	//bearish crossover
	else if (shortma[index] < longma[index] && lastShort > lastLong)
	{
		return 1;
	}
		return 0;
	return 0;
}

extern "C" algorithm_data process(const std::vector<std::vector<double>>& candle_data, unsigned int index, unsigned int window)
{
	indicator_dataset dataset;
	candleset candles;
	candles.resize(candle_data.size());
	for (unsigned int i = 0; i < candles.size(); i++)
	{
		//candles[i] = candle_data[i];
	}
	
	dataset["longma"] = longma_INDICATOR.calculate(candles, index, window);
	dataset["shortma"] = shortma_INDICATOR.calculate(candles, index, window);
	unsigned int action = getAction(dataset, candles, index);
	return { dataset, action };
}

extern "C" std::string getName()
{
	return "Simple MA";
}

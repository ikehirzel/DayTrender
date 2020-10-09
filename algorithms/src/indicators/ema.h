#pragma once

#include "indicator.h"

	class EMA: public Indicator
	{
	public:
		EMA(unsigned int denom) : Indicator("EMA", denom) {}

		indicator_data calculate(const daytrender::candleset& candles, unsigned int index,
			unsigned int window)
		{
			unsigned int startIndex, range;

			range = window / denom;
			startIndex = (index + 1) - window;

			std::vector<double> data(window);

			if(index >= candles.size() || startIndex >= candles.size())
			{
				std::cout << "EMA: index out of bounds" << std::endl;
				return { name, data };
			}

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
		return { name, data };
	}
};

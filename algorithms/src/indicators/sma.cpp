#include "sma.h"

#define SMA_NAME "SMA"

namespace daytrender
{
	SMA::SMA(unsigned int denom) : Indicator(SMA_NAME, denom) {}
	
	indicator_data SMA::calculate(const candleset& candles, unsigned int index,
		unsigned int window)
	{
		unsigned int startIndex, range;
		
		startIndex = (index + 1) - window;
		range = window / denom;
		
		std::vector<double> data(window);
		
		if(index >= candles.size() || startIndex >= candles.size())
		{
			std::cout << "SMA: index out of bounds" << std::endl;
			return { name, data };
		}
		
		double sum = 0.0;
		unsigned int dataIndex = 0;
		
		//when the dataIndex == range - 1, we can start to do the actual calculation
		for(unsigned int i = startIndex; i < startIndex + range; i++)
		{
			sum += candles[i].close;
			data[dataIndex] = sum / (double)(dataIndex + 1);
			dataIndex++;
		}
		
		for(unsigned int i = startIndex + range; i <= index; i++)
		{
			sum = 0.0;
			unsigned int si = (i + 1) - range;
			int count  = 0;
			for(unsigned int j = si; j <= i; j++)
			{
				sum += candles[j].close;
				count++;
			}
			
			data[dataIndex] = sum / (double)range;
			dataIndex++;
		}
		unsigned int diff = window - range;
		printf("SMA: %d\n", diff);
		
		return { name, data };
	}
}
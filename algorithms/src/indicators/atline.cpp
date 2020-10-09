#include "atline.h"

#include "../line.h"

#define ATLINE_NAME "Average Trend Line"

namespace daytrender
{
	AvgTrendLine::AvgTrendLine(unsigned int denom) : Indicator(ATLINE_NAME, denom) {}
	
	indicator_data AvgTrendLine::calculate(const candleset& candles, unsigned int index,
		unsigned int window)
	{	
		unsigned int startIndex, range;
		
		range = window / denom;
		startIndex = (index + 1) - window;
	
		std::vector<double> data(window);
	
		Line line;
		
		
		if(index >= candles.size() || startIndex >= candles.size())
		{
			std::cout << "AvgTrendLine: index out of bounds" << std::endl;
			line = Line(0, 0);
			return { name, data };
		}
		else
		{
			line.toBestFit(candles, index, range);
		}
		for(unsigned int i = 0; i < window; i++)
		{
			data[i] = line[i];
		}
		return { name, data };
	}
	
}
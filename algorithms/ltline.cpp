#include "ltline.h"

#include "../line.h"

#define LTLINE_NAME "Lower Trend Line"

namespace daytrender
{
	LowerTrendLine::LowerTrendLine(unsigned int denom) : Indicator(LTLINE_NAME, denom) {}
	
	indicator_data LowerTrendLine::calculate(const candleset& candles, unsigned int index,
		unsigned int window)
	{
		unsigned int startIndex, range;
		
		range = window / denom;
		startIndex = (index + 1) - window;
		
		std::vector<double> data(window);
		
		Line line;
		
		
		if(index >= candles.size() || startIndex >= candles.size())
		{
			std::cout << "LowerTrendLine: index out of bounds" << std::endl;
			line = Line(0, 0);
			return { name, data };
		}
		else
		{
			Line mid(candles, index, window);
			std::vector<Point> points;
			
			double maxDiff = 0.0;
			
			for(unsigned int i = startIndex; i <= index; i++)
			{
				if(candles[i].close < mid[i])
				{
					double diff = mid[i] - candles[i].close;
					
					if(diff > maxDiff)
					{
						maxDiff = diff;
					}
				}
			}
			
			double thresh = maxDiff / 2.0;
			
			
			for(unsigned int i = startIndex + 1; i <= index; i++)
			{
				candle last = candles[i - 1], current = candles[i];
				
				//trough
				//if(current.movement == BEARISH && current.open > mid[i] && last.open < current.open && last.close - mid[i - 1] >= thresh)
				if(current.movement == BULLISH && current.open < mid[i] && last.open > current.open && mid[i - 1] - last.close >= thresh)
				{
					Point p(i, last.close);
					points.push_back(p);
				}
			}

			line.toBestFit(points);
		}
		
		for(unsigned int i = 0; i < window; i++)
		{
			data[i] = line[i];
		}
		
		return { name, data };
	}
}
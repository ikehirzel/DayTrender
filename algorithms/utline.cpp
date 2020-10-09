#include "utline.h"

#include "../line.h"

#define UTLINE_NAME "Upper Trend Line"

namespace daytrender
{
	UpperTrendLine::UpperTrendLine(unsigned int denom) : Indicator(UTLINE_NAME, denom) {}
	
	indicator_data UpperTrendLine::calculate(const candleset& candles, unsigned int index,
		unsigned int window)
	{
		unsigned int startIndex, range;
		
		startIndex = (index + 1) - window;
		range = window / denom;
		
		std::vector<double> data(window);
		
		Line line;
		
		
		if(index >= candles.size() || startIndex >= candles.size())
		{
			std::cout << "UpperTrendLine: index out of bounds" << std::endl;
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
				if(candles[i].close > mid[i])
				{
					double diff = candles[i].close - mid[i];
					if(diff > maxDiff)
					{
						maxDiff = diff;
					}
				}
			}
			
			double thresh = maxDiff / 2.0;
			
			for(unsigned int i = startIndex + 2; i < index; i++)
			{
				candle lastLast = candles[i - 2], last = candles[i - 1], current = candles[i];
				
				//crest
				if(current.movement == BEARISH && current.open > mid[i] && last.open < current.open && lastLast.close <  last.close)
				{
					Point p(i, last.close);
					points.push_back(p);
				}
			}
			Point p(index, candles[index].close);
			points.push_back(p);
			
			
			line.toBestFit(points);
		}
		
		for(unsigned int i = 0; i < window; i++)
		{
			data[i] = line[i];
		}
		
		return { name, data };
	}
}
#pragma once

#include <iostream>
#include <vector>

namespace daytrender
{
	
	struct candle;
	typedef std::vector<candle> candleset;
	
	struct Point
	{
		double x, y;
		Point(double x, double y)
		{
			this->x = x;
			this->y = y;
		}
	};
	

	struct Line
	{
		double m = 0.0L, b = 0.0L;
		
		Line() = default;
		Line(double m, double b);
		Line(const std::vector<Point>& points);
		Line(const candleset& candles, unsigned int index, unsigned int window);
		void toBestFit(const std::vector<Point>& points);
		void toBestFit(const candleset& candles, unsigned int index, unsigned int window);
		double operator[](int x) const;
		
		friend std::ostream& operator<<(std::ostream& out, const Line& line);
	};	
}
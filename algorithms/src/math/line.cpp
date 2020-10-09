#include "line.h"

#include "candle.h"

namespace daytrender
{
	Line::Line(double m, double b)
	{
		this->m = m;
		this->b = b;	
	}
	
	Line::Line(const std::vector<Point>& points)
	{
		toBestFit(points);
	}
	
	Line::Line(const candleset& candles, unsigned int index, unsigned int window)
	{
		toBestFit(candles, index, window);
	}
	
	void Line::toBestFit(const std::vector<Point>& points)
	{
		double xSum = 0.0L, ySum = 0.0L, xySum = 0.0L,
			x2Sum = 0.0L, length = (double)points.size();
			
		for(unsigned int i = 0; i < points.size(); i++)
		{
			double x = points[i].x;
			double y = points[i].y;
			xSum += x;
			ySum += y;
			xySum += (x * y);
			x2Sum += (x * x);
		}
		m = (length * xySum - xSum * ySum) / (length * x2Sum - (xSum * xSum));
		b = (ySum - m * xSum) / length;
	}
	
		
	void Line::toBestFit(const candleset& candles, unsigned int index, unsigned int window)
	{
		//std::cout << "LINE INDEX: " << index << std::endl;
		unsigned int start = (index + 1) - window;
		
		if(index > candles.size() - 1 || start > candles.size() - 1)
		{
			std::cout << "Line: index out of bounds!" << std::endl;
			m = 0;
			b = 0;
			return;
		}
		
		double xSum = 0.0L, ySum = 0.0L, xySum = 0.0L,
			x2Sum = 0.0L, length = (double)window;
		
		for(unsigned int i = start; i <= index; i++)
		{
			double x = (double)i;
			double y = candles[i].close;
			xSum += x;
			ySum += y;
			xySum += (x * y);
			x2Sum += (x * x);
		}
		m = (length * xySum - xSum * ySum) / (length * x2Sum - (xSum * xSum));
		b = (ySum - m * xSum) / length;
	}
	
	double Line::operator[](int x) const
	{
		return m * x + b;
	}
		
	std::ostream& operator<<(std::ostream& out, const Line& line)
	{
		out << line.m << "X + " << line.b;
		return out;
	}	
}
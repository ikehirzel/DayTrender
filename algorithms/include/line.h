#pragma once
#include <candle.h>
#include <vector>

namespace daytrender
{
	struct Point
	{
		double x, y;
		Point() = default;
		Point(double _x, double _y) { x = _x; y = _y; }
	};

	struct Line
	{
		double m = 0.0, b = 0.0;
		Line() = default;
		Line(double _m, double _b) { m = _m; b = _b; }
		double operator[](int x) const { return m * x + b; }
	};

	Line toBestFit(const std::vector<Point>& points)
	{
		double m, b, xSum = 0.0L, ySum = 0.0L, xySum = 0.0L,
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
		return { m, b };
	}

	Line toBestFit(const candleset& candles)
	{
		double m, b, xSum = 0.0L, ySum = 0.0L, xySum = 0.0L,
			x2Sum = 0.0L, length = candles.size();

		for(unsigned int i = 0; i < candles.size(); i++)
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
		return { m, b };
	}
}

#include "candle.h"
#include <iostream>
#include <cmath>

namespace daytrender
{
	void printCandle(const candle& c)
	{
		std::cout << "candle:\n{\n\tOpen   :\t" << c[0] << "\n\tHigh   :\t" << c[1] << "\n\tLow    :\t" << c[2] << "\n\tClose  :\t" << c[3] << "\n\tVolume :\t" << c[4] << "\n}";
	}

	void calculateCandle(candle& c)
	{
		if (c.size() < 4)
		{
			std::cout << "Cannot calculate candle!" << std::endl;
			return;
		}

		c.resize(8);
		c[5] = c[3] - c[0];
		c[6] = c[5] / abs(c[5]);
		c[7] = (c[1] - c[2]) / (abs(c[0] - c[3]) + 1);
	}
}
#include <cmath>

namespace daytrender
{
	inline double get_shares_to_order(double buying_power, double price, double order_minimum,
		double fee)
	{
		return std::floor(((buying_power / (1.0 + fee)) / price) / order_minimum) * order_minimum;
	}
}
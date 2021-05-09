#include <data/position.h>

namespace daytrender
{
	Position::Position(double amt_invested, double fee, double minimum, double price, double shares) :
		_amt_invested(amt_invested),
		_fee(fee),
		_minimum(minimum),
		_price(price),
		_shares(shares) {}
}
#include "assetinfo.h"

namespace daytrender
{
	AssetInfo::AssetInfo(double fee, double price, double shares, double minimum)
	{
		_fee = fee;
		_price = price;
		_shares = shares;
		_minimum = minimum;
	}
}
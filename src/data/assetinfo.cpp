#include "assetinfo.h"

namespace daytrender
{
	AssetInfo::AssetInfo(double fee, double price, double shares)
	{
		_fee = fee;
		_price = price;
		_shares = shares;
	}
}
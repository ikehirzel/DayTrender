#include "accountinfo.h"

namespace daytrender
{
	AccountInfo::AccountInfo(double balance, double buying_power, double equity, int leverage, bool shorting_enabled)
	{
		_balance = balance;
		_buying_power = buying_power;
		_equity = equity;
		_leverage = leverage;
		_shorting_enabled = shorting_enabled;
	}

	AccountInfo::AccountInfo(const AccountInfo& other, double base_equity, double risk, int asset_count)
	{
		*this = other;
		_pl = _equity - base_equity;
		_bp_per_asset = (risk * _buying_power) / (double)asset_count;
	}
}
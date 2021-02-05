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
}
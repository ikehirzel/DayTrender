#include <data/account.h>

namespace daytrender
{
	Account::Account(double balance, double buying_power, double margin_used, double equity,
		int leverage, bool shorting_enabled) :
		_balance(balance), _buying_power(buying_power), _margin_used(margin_used), _equity(equity),
		_leverage(leverage), _shorting_enabled(shorting_enabled) {}
}
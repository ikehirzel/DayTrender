#pragma once

namespace daytrender
{
	struct AccountInfo
	{
		double balance = 0.0;
		double buying_power = 0.0;
		double equity = 0.0;
		double leverage = 0.0;
		double money_per_share = 0.0;
		bool shorting_enabled = false;
		double pl = 0.0;
	};
}

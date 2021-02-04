#include "paperaccount.h"

#include <cmath>

namespace daytrender
{
	PaperAccount::PaperAccount(double principal, int leverage, double fee, double order_minimum,
		double initial_price, bool shorting_enabled, int interval, const std::vector<int>& ranges)
	{
		_principal = principal;
		_balance = principal;
		_leverage = (double)leverage;
		_fee = fee;
		_order_minimum = order_minimum;
		_price = initial_price;
		_shorting_enabled = shorting_enabled;
		_interval = interval;
		_ranges = ranges;
	}

	bool PaperAccount::enter_long()
	{
		if (_shares < 0.0)
		{
			if (!exit_short()) return false;
		}

		_long_entrances++;
		// if shorting, exit the short position before entering long position

		double fee_mult = 1.0 + _fee;
		double shares_to_order = std::floor(((buying_power() / fee_mult) / _price) / _order_minimum) * _order_minimum;

		_margin_used += shares_to_order * _price * fee_mult;
		_shares += shares_to_order;

		return true;
	}

	bool PaperAccount::exit_long()
	{
		if (_shares <= 0.0) return true;

		_long_exits++;

		// the avg price paid for each share as well as the fee
		double fee_mult = 1.0 + _fee;
		double returns = _margin_used - (_shares * _price * fee_mult);
		_balance += returns;

		if (returns > 0.0)
		{
			_long_wins++;
			_long_profits += returns;
		}
		else
		{
			_long_losses -= returns;
		}

		_margin_used = 0;
		_shares = 0;

		if (_balance < 0.0)
		{
			_error = "resolution of long exit caused balance to go negative: " + std::to_string(_balance);
			return false;
		}

		return true;
	}
	
	bool PaperAccount::enter_short()
	{
		
		// regardless of whether shorting is enabled or not, exit long position
		if (_shares > 0.0)
		{
			if (!exit_long()) return false;
		}

		if (!_shorting_enabled) return true;

		_short_entrances++;

		// calculation for short buying
		double fee_mult = 1.0 + _fee;
		double shares_to_short = std::floor(((buying_power() / fee_mult) / _price) / _order_minimum) * _order_minimum;

		_margin_used += shares_to_short * _price * fee_mult;
		_shares -= shares_to_short;

		return true;
	}

	bool PaperAccount::exit_short()
	{
		if (_shares >= 0.0) return true;
		_short_exits++;

		// the avg price paid for each share as well as the fee
		double fee_mult = 1.0 + _fee;
		double returns = _margin_used + _shares * _price * fee_mult;
		_balance += returns;

		if (returns > 0.0)
		{
			_short_wins++;
			_short_profits += returns;
		}
		else
		{
			_short_losses -= returns;
		}

		_margin_used = 0.0;
		_shares = 0;

		if (_balance < 0.0)
		{
			_error = "resolution of short exit caused balance to go negative: " + std::to_string(_balance);
			return false;
		}

		return true;
	}

	double PaperAccount::net_return() const
	{
		return equity() - _principal;
	}

	double PaperAccount::pct_return() const
	{
		if(_principal > 0.0)
		{
			return net_return() / _principal;
		}
		return 0.0;
	}

	double PaperAccount::elapsed_hours() const
	{
		return (double)(_interval * _updates) / 3600.0;
	}

	double PaperAccount::net_per_year() const
	{
		return _principal * (1.0 + pct_per_year()) - _principal;
	}

	double PaperAccount::pct_per_year() const
	{
		if (_updates > 0)
		{
			return std::pow(1.0 + pct_return(), 8760.0 / elapsed_hours()) - 1.0;
		}
		return 0.0;
	}

	double PaperAccount::long_win_rate() const
	{
		if (_long_exits > 0)
		{
			return (double)_long_wins / (double)_long_exits;
		}
		return 0.0;
	}

	double PaperAccount::long_profit_rate() const
	{
		if (long_movement() > 0)
		{
			return _long_profits / long_movement();
		}
		return 0.0;
	}

	double PaperAccount::short_win_rate() const
	{
		if (_short_exits > 0)
		{
			return (double)_short_wins / (double)_short_exits;
		}
		return 0.0;
	}

	double PaperAccount::short_profit_rate() const
	{
		if (short_movement() > 0)
		{
			return _short_profits / short_movement();
		}
		return 0.0;
	}

	std::string PaperAccount::to_string() const
	{
		std::string out;

		out = "PaperAccount:\n{";
		out += "\n    L Entrances :    " + std::to_string(_long_entrances);
		out += "\n    L Exits     :    " + std::to_string(_long_exits);
		out += "\n    S Entrances :    " + std::to_string(_short_entrances);
		out += "\n    S Exits     :    " + std::to_string(_short_exits);
		out += "\n    Interval    :    " + std::to_string(_interval);
		out += "\n    Ranges      :    ";
		for (int i = 0; i < _ranges.size(); i++)
		{
			if (i > 0) out += ", ";
			out += std::to_string(_ranges[i]);
		}
		out += "\n    Elapsed Hrs :    " + std::to_string(elapsed_hours()) + " (" + std::to_string(elapsed_hours() / 24.0) + " days)";
		out += "\n";
		out += "\n    Principal   :  $ " + std::to_string(_principal);
		out += "\n    Shares      :    " + std::to_string(_shares);
		out += "\n    Balance     :  $ " + std::to_string(_balance);
		out += "\n    Equity      :  $ " + std::to_string(equity());
		out += "\n    Buy Power   :  $ " + std::to_string(buying_power());
		out += "\n    Leverage    :  x " + std::to_string(leverage());
		out += "\n    Fee         :  % " + std::to_string(_fee);
		out += "\n    Order Min   :    " + std::to_string(_order_minimum);
		out += "\n";
		out += "\n    L Profits   :  $ " + std::to_string(_long_profits);
		out += "\n    L Losses    :  $ " + std::to_string(_long_losses);
		out += "\n    S Profits   :  $ " + std::to_string(_short_profits);
		out += "\n    S Losses    :  $ " + std::to_string(_short_losses);
		out += "\n    Net Return  :  $ " + std::to_string(net_return());
		out += "\n    Pct Return  :  % " + std::to_string(pct_return() * 100.0);
		out += "\n";
		out += "\n    Net / Year  :  $ " + std::to_string(net_per_year());
		out += "\n    Pct / Year  :  % " + std::to_string(pct_per_year() * 100.0);
		out += "\n";
		out += "\n    L Win Rate  :  % " + std::to_string(long_win_rate() * 100.0);
		out += "\n    L Prft Rate :  % " + std::to_string(long_profit_rate() * 100.0);
		out += "\n    S Win Rate  :  % " + std::to_string(short_win_rate() * 100.0);
		out += "\n    S Prft Rate :  % " + std::to_string(short_profit_rate() * 100.0);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		out << acc.to_string();
		return out;
	}
}
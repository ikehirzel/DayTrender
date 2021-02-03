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
		_buys++;

		double money_available = buying_power() / (1.0 + _fee);
		double max_shares = money_available / _price;
		double shares_to_order = std::floor(max_shares / _order_minimum) * _order_minimum - _shares;

		_margin_used += shares_to_order * _price * (_fee + 1.0);
		_shares += shares_to_order;

		return true;
	}

	bool PaperAccount::exit_long()
	{		
		_sales++;
		// the avg price paid for each share as well as the fee
		double return_per_share = _price * (1.0 - _fee);
		double avg_cost = _margin_used / _shares;
		if (return_per_share > avg_cost) _long_wins++;

		// actual calculations
		double returns = _shares * return_per_share;
		_margin_used -= returns;
		_balance -= _margin_used;

		if (_margin_used < 0.0)
		{
			_long_profits -= _margin_used;
		}
		else
		{
			_long_losses += _margin_used;
		}

		_margin_used = 0;
		_shares = 0;

		if (_balance < 0.0)
		{
			_error = "application of returns caused balance to go negative: " + std::to_string(_balance);
			return false;
		}

		return true;
	}

	bool PaperAccount::enter_short()
	{
		if (!_shorting_enabled) return true;
		return false;
	}

	bool PaperAccount::exit_short()
	{
		if (!_shorting_enabled) return true;
		return false;
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
		if (trades() > 0)
		{
			return (double)_long_wins / (double)_sales;
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

	std::string PaperAccount::to_string() const
	{
		std::cout << "sales: " << _sales << std::endl;

		std::string out;
		out = "PaperAccount:\n{";
		out += "\n    Buys        :    " + std::to_string(_buys);
		out += "\n    Sales       :    " + std::to_string(_sales);
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
		out += "\n    Net Return  :  $ " + std::to_string(net_return());
		out += "\n    Pct Return  :  % " + std::to_string(pct_return() * 100.0);
		out += "\n";
		out += "\n    Net / Year  :  $ " + std::to_string(net_per_year());
		out += "\n    Pct / Year  :  % " + std::to_string(pct_per_year() * 100.0);
		out += "\n";
		out += "\n    Long W Rate :  % " + std::to_string(long_win_rate() * 100.0);
		out += "\n    Long P Rate :  % " + std::to_string(long_profit_rate() * 100.0);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		out << acc.to_string();
		return out;
	}
}
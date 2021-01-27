#include "paperaccount.h"

#include <cmath>

namespace daytrender
{
	PaperAccount::PaperAccount(double principal, int leverage, double fee, double minimum,
		double initial_price, int interval, const std::vector<int>& ranges)
	{
		_principal = principal;
		_leverage = (double)leverage;
		_balance = principal;
		_fee = fee;
		_minimum = minimum;
		_price = initial_price;
		_last_act_price = initial_price;
		_interval = interval;
		_ranges = ranges;
	}

	bool PaperAccount::buy(double shares)
	{
		if (shares < _minimum)
		{
			_error = "attempt to buy " + std::to_string(shares) + " shares with trade minimum of " + std::to_string(_minimum);
			return false;
		}

		double cost = shares * _price * (_fee + 1.0);
		
		if (cost > buying_power())
		{			
			_error = "attempt to buy " + std::to_string(shares) + " shares @ $" + std::to_string(cost) + "/share ($" + std::to_string(cost) + ") with insufficient buying power: $" + std::to_string(buying_power());
			return false;
		}
		
		// updating action to price to see if is loss

		_buys++;
		if (_price > _last_act_price)
		{
			_buy_losses++;
		}
		else if (_price < _last_act_price)
		{
			_buy_wins++;
		}
		_last_act_price = _price;
		
		// actual calculations

		_margin_used += cost;
		_shares += shares;

		return true;
	}

	bool PaperAccount::sell(double shares)
	{
		if (shares < _minimum)
		{
			_error = "attempt to sell " + std::to_string(shares) + " shares with trade minimum of " + std::to_string(_minimum);
			return false;
		}

		if (shares > _shares)
		{
			_error = "attempt to sell " + std::to_string(shares) + " shares with insufficient amount: " + std::to_string(_shares);
			return false;
		}
		
		_sales++;
		if (_price > _last_act_price)
		{
			_sale_wins++;
		}
		else if (_price < _last_act_price)
		{
			_sale_losses++;
		}
		_last_act_price = _price;

		// actual calculations
		double returns = shares * _price * (1.0 - _fee);
		double margin_change = _margin_used * (shares / _shares);
		_margin_used -= margin_change;
		_balance -= (margin_change - returns);
		_shares -= shares;

		if (_balance < 0.0)
		{
			_error = "application of returns caused balance to go negative: " + std::to_string(_balance);
			return false;
		}
		// need to make sure that once the shares close out, it undoes the margin, this is wrong
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

	double PaperAccount::buy_win_rate() const
	{
		if(_buys > 0)
		{
			return (double)_buy_wins / (double)_buys;
		}
		return 0.0;
	}

	double PaperAccount::buy_loss_rate() const
	{
		if (_buys > 0)
		{
			return (double)_buy_losses / (double)_buys;
		}
		return 0.0;
	}

	double PaperAccount::sale_win_rate() const
	{
		if (_sales > 0)
		{
			return (double)_sale_wins / (double)_sales;
		}
		return 0.0;
	}

	double PaperAccount::sale_loss_rate() const
	{
		if (_sales > 0)
		{
			return (double)_sale_losses / (double)_sales;
		}
		return 0.0;
	}

	double PaperAccount::win_rate() const
	{
		if	(trades() > 0)
		{
			return (double)(_buy_wins + _sale_wins) / (double)trades();
		}
		return 0.0;
	}

	double PaperAccount::loss_rate() const
	{
		if (trades() > 0)
		{
			return (double)(_buy_losses + _sale_losses) / (double)trades();
		}
		return 0.0;
	}

	std::string PaperAccount::to_string() const
	{
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
		out += "\n    Fee         :  % " + std::to_string(fee());
		out += "\n    Minimum     :    " + std::to_string(minimum());
		out += "\n";
		out += "\n    Net Return  :  $ " + std::to_string(net_return());
		out += "\n    % Return    :  % " + std::to_string(pct_return() * 100.0);
		out += "\n";
		out += "\n    Net / Year  :  $ " + std::to_string(net_per_year());
		out += "\n    % / Year    :  % " + std::to_string(pct_per_year() * 100.0);
		out += "\n";
		out += "\n    Win Rate    :  % " + std::to_string(win_rate() * 100.0);
		out += "\n    B Win Rate  :  % " + std::to_string(buy_win_rate() * 100.0);
		out += "\n    S Win Rate  :  % " + std::to_string(sale_win_rate() * 100.0);
		out += "\n";
		out += "\n    Loss Rate   :  % " + std::to_string(loss_rate() * 100.0);
		out += "\n    B Loss Rate :  % " + std::to_string(buy_loss_rate() * 100.0);
		out += "\n    S Loss Rate :  % " + std::to_string(sale_loss_rate() * 100.0);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		out << acc.to_string();
		return out;
	}
}
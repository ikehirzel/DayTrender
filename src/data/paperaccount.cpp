#include "paperaccount.h"

#include <hirzel/fountain.h>

namespace daytrender
{
	PaperAccount::PaperAccount(double initial, double fee, double minimum, int interval,
		const std::vector<int>& ranges)
	{
		_initial = initial;
		_balance = initial;
		_fee = fee;
		_minimum = minimum;
		_interval = interval;
		_ranges = ranges;
	}

	void PaperAccount::buy(double shares)
	{
		if (shares < _minimum)
		{
			warningf("A minimum of %f must be bought to complete a trade");
			return;
		}

		if(!_price)
		{
			errorf("Price must be set before purchasing!");
			return;
		}
		
		if (_price > _last_act_price)
		{
			_buy_losses++;
		}
		else if (_price < _last_act_price)
		{
			_buy_wins++;
		}
		
		double cost = _shares * _price * (_fee + 1.0);
		
		if (cost > _balance)
		{			
			warningf("Not enough balance to complete purchase");
			return;
		}
		
		_last_act_price = _price;
		_buys++;
		_shares += shares;
		_balance -= cost;
	}

	void PaperAccount::sell(double shares)
	{
		if (shares < _minimum)
		{
			warningf("A minimum of %f myst be sold to complete trade", _minimum);
			return;
		}

		if (shares > _shares)
		{
			warningf("Not enough shares to complete sale");
			return;
		}

		if(!_price)
		{
			errorf("Price must be set before selling!");
			return;
		}

		
		if (_price > _last_act_price)
		{
			_sale_wins++;
		}
		else if (_price < _last_act_price)
		{
			_sale_losses++;
		}

		double returns = _shares * _price * (1.0 - _fee);
		
		_last_act_price = _price;
		_sales++;
		shares -= _shares;
		_balance += returns;
	}	

	double PaperAccount::equity() const
	{
		return _balance + (_shares * _price);
	}

	double PaperAccount::net_return() const
	{
		return equity() - _initial;
	}

	double PaperAccount::pct_return() const
	{
		if(_initial > 0.0)
		{
			return net_return() / _initial;
		}
		return 0.0;
	}

	double PaperAccount::elapsed_hours() const
	{
		return (double)(_interval * _updates) / 3600.0;
	}

	double PaperAccount::avg_net_per_hour() const
	{
		double hours = elapsed_hours();
		if(hours > 0.0)
		{
			return net_return() / hours;
		}
		return 0.0;
	}

	double PaperAccount::avg_pct_per_hour() const
	{
		double hours = elapsed_hours();
		if(hours > 0.0)
		{
			return pct_return() / hours;
		}
		return 0.0;
	}

	double PaperAccount::buy_win_rate() const
	{
		if(_buys)
		{
			return (double)_buy_wins / (double)(_buys);
		}
		return 0.0;
	}
	double PaperAccount::sale_win_rate() const
	{
		if (_sales)
		{
			return (double)_sale_wins / (double)_sales;
		}
		return 0.0;
	}

	double PaperAccount::win_rate() const
	{
		double base = (double)trades();
		if	(base > 0.0)
		{
			return (double)(_buy_wins + _sale_wins) / base;
		}
		return 0.0;
	}

	std::string PaperAccount::to_string() const
	{
		std::string out;
		out = "PaperAccount:\n{";
		out += "\n    Buys        :    " + std::to_string(_buys);
		out += "\n    Sells       :    " + std::to_string(_sales);
		out += "\n    Interval    :    " + std::to_string(_interval);
		out += "\n    Ranges      :    ";
		for (int i = 0; i < _ranges.size(); i++)
		{
			if (i > 0) out += ", ";
			out += std::to_string(_ranges[i]);
		}
		out += "\n    Elapsed Hrs :    " + std::to_string(elapsed_hours()) + " (" + std::to_string(elapsed_hours() / 24.0) + " days)";
		out += "\n";
		out += "\n    Initial     :  $ " + std::to_string(_initial);
		out += "\n    Shares      :    " + std::to_string(_shares);
		out += "\n    Balance     :  $ " + std::to_string(_balance);
		out += "\n    Equity      :  $ " + std::to_string(equity());
		out += "\n";
		out += "\n    Net Return  :  $ " + std::to_string(net_return());
		out += "\n    % Return    :  % " + std::to_string(pct_return() * 100.0);
		out += "\n";
		out += "\n    Hr Return   :  $ " + std::to_string(avg_net_per_hour());
		out += "\n    Hr% Return  :  % " + std::to_string(avg_pct_per_hour() * 100.0);
		out += "\n";
		out += "\n    Win Rate    :  % " + std::to_string(win_rate() * 100.0);
		out += "\n    B Win Rate  :  % " + std::to_string(buy_win_rate() * 100.0);
		out += "\n    S Win Rate  :  % " + std::to_string(sale_win_rate() * 100.0);
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		out << acc.to_string();
		return out;
	}
}
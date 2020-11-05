#include "paperaccount.h"

#include "../data/interval.h"
#include "../data/action.h"
#include <cmath>
#include <hirzel/fountain.h>

namespace daytrender
{
	PaperAccount::PaperAccount(double initial, double fee, double minimum, unsigned int interval,
		unsigned int window)
	{
		this->initial = initial;
		this->balance = initial;
		this->fee = fee;
		this->minimum = minimum;
		this->interval = interval;
		this->window = window;
	}

	void PaperAccount::buy(double _shares)
	{
		if (shares < minimum)
		{
			warningf("A minimum of %f must be bought to complete a trade");
			return;
		}
		
		if(!price)
		{
			errorf("Price must be set before purchasing!");
			return;
		}

		if(!lastActPrice) lastActPrice = price;
		
		if (price > lastActPrice)
		{
			buylosses++;
		}
		else if (price < lastActPrice)
		{
			buywins++;
		}
		
		double cost = _shares * price * (fee + 1.0);
		
		if (cost > balance)
		{			
			warningf("Not enough balance to complete purchase");
			return;
		}
		
		buys++;
		shares += _shares;
		balance -= cost;
	}

	void PaperAccount::sell(double _shares)
	{
		if (_shares < minimum)
		{
			warningf("A minimum of %f myst be sold to complete trade", minimum);
			return;
		}

		if (_shares > shares)
		{
			warningf("Not enough shares to complete sale");
			return;
		}

		if(!price)
		{
			errorf("Price must be set before selling!");
			return;
		}

		if(!lastActPrice) lastActPrice = price;
		
		if (price > lastActPrice)
		{
			sellwins++;
		}
		else if (price < lastActPrice)
		{
			selllosses++;
		}

		double returns = _shares * price * (1.0 - fee);
		
		sells++;
		shares -= _shares;
		balance += returns;
	}	

	double PaperAccount::equity() const
	{
		return balance + (shares * price);
	}

	double PaperAccount::netReturn() const
	{
		return equity() - initial;
	}

	double PaperAccount::percentReturn() const
	{
		if(initial > 0.0)
		{
			return netReturn() / initial;
		}
		return 0.0;
	}

	double PaperAccount::elapsedHours() const
	{
		return (double)(interval * updates) / 3600.0;
	}

	double PaperAccount::avgHourNetReturn() const
	{
		double hours = elapsedHours();
		if(hours > 0.0)
		{
			return netReturn() / hours;
		}
		return 0.0;
	}

	double PaperAccount::avgHourPercentReturn() const
	{
		double hours = elapsedHours();
		if(hours > 0.0)
		{
			return percentReturn() / hours;
		}
		return 0.0;
	}

	double PaperAccount::buyWinRate() const
	{
		if(buys)
		{
			return (double)buywins / (double)(buys);
		}
		return 0.0;
	}
	double PaperAccount::sellWinRate() const
	{
		if (sells)
		{
			return (double)sellwins / (double)sells;
		}
		return 0.0;
	}

	double PaperAccount::winRate() const
	{
		double base = (double)(buys + sells);
		if	(base > 0.0)
		{
			return (double)(buywins + sellwins) / base;
		}
		return 0.0;
	}

	std::string PaperAccount::to_string() const
	{
		std::string out;
		out = "PaperAccount:\n{";
		out += "\n    Buys        :    " + std::to_string(buys);
		out += "\n    Sells       :    " + std::to_string(sells);
		out += "\n    Interval    :    " + std::to_string(interval);
		out += "\n    Window      :    " + std::to_string(window);
		out += "\n    Elapsed Hrs :    " + std::to_string(elapsedHours());
		out += "\n";
		out += "\n    Initial     :    " + std::to_string(initial);
		out += "\n    Shares      :    " + std::to_string(shares);
		out += "\n    Balance     :    " + std::to_string(balance);
		out += "\n    Equity      :    " + std::to_string(equity());
		out += "\n";
		out += "\n    Net Return  :    " + std::to_string(netReturn());
		out += "\n    % Return    :    " + std::to_string(percentReturn() * 100.0) + " %";
		out += "\n";
		out += "\n    Hr Return   :    " + std::to_string(avgHourNetReturn());
		out += "\n    Hr% Return  :    " + std::to_string(avgHourPercentReturn() * 100.0) + " %";
		out += "\n";
		out += "\n    Win Rate    :    " + std::to_string(winRate()) + " %";
		out += "\n    B Win Rate  :    " + std::to_string(buyWinRate()) + " %";
		out += "\n    S Win Rate  :    " + std::to_string(sellWinRate()) + " %";
		out += "\n}";
		return out;
	}

	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		out << acc.to_string();
		return out;
	}
}
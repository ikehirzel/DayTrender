#include "paperaccount.h"

#include "../data/interval.h"
#include "../data/action.h"
#include <cmath>

namespace daytrender
{
	PaperAccount::PaperAccount(double initial, double fee, double minimum,
		unsigned int interval, unsigned int window)
	{
		this->initial = initial;
		this->balance = initial;
		this->fee = fee;
		this->minimum = minimum;
		this->interval = interval;
		this->window = window;
		
		prices.clear();
		actions.clear();
	}
	
	PaperAccount::PaperAccount(const PaperAccount& other)
	{
		*this = other;
	}
	
	PaperAccount::PaperAccount(const PaperAccount& other, unsigned int interval, unsigned int window)
	{
		*this = other;
		this->interval = interval;
		this->window = window;
	}

	void PaperAccount::buy(const double& shares)
	{
		std::string name = "PaperAccount::buy(shares): ";
		if (shares < minimum)
		{
			std::cout << name + "A minimum of " << minimum
				<< " shares must be bought to complete trade!" << std::endl;
			return;
		}
		
		double price = getPrice();
		
		if(!price)
		{
			std::cout << "\t" + name + "Price returned null!\n";
			return;
		}
		
		double cost = shares * price * (fee + 1.0);
		
		if (cost > balance)
		{			
			std::cout << name + "Not enough balance to complete purchase!\nBalance: "
				<< balance << std::endl;
			std::cout << "Cost: " << cost << "\nBalance: " << balance
				<< "\nFee: " << fee << "\nShares: " << shares << std::endl;
				std::cin.get();
			return;
		}
		
		actions.back() = ACTION_BUY;
		buys++;
		this->shares += shares;
		balance -= cost;
	}

	void PaperAccount::sell(const double& shares)
	{
		std::string name = "PaperAccount::sell(shares): ";
		bool evac = false;
		double price = getPrice();
		if(!price)
		{
			std::cout << "\t" + name + "Price returned null!\n";
			return;
		}
		double returns = shares * price * (1.0 - fee);
		if (shares > this->shares)
		{
			std::cout << name << "Not enough shares to complete sale!\nShares: " << shares << std::endl;
			evac = true;
		}

		if (shares < minimum)
		{
			std::cout << name << "A minimum of " << minimum << " shares must be sold to complete trade!" << std::endl;
			evac = true;
		}
		if (evac)
		{
			return;
		}
		else
		{
			actions.back() = ACTION_SELL;
			sells++;
			this->shares -= shares;
			balance += returns;
		}
	}

	double PaperAccount::equity() const
	{
		std::string name = "PaperAccount::equity(): ";
		double price = getPrice();
		if(!price)
		{
			std::cout << "\t" + name + "Price returned null!\n";
			return 0.0;
		}
		return balance + (shares * price);
	}

	double PaperAccount::netReturn() const
	{
		std::string name = "PaperAccount::netReturn(): ";
		double eq = equity();
		if(!eq)
		{
			std::cout << "\t\t" << name << "equity() returned null!\n";
			return 0.0;
		}
		return eq - initial;
	}

	double PaperAccount::percentReturn() const
	{
		std::string name = "PaperAccount::percentReturn(): ";
		double net_return = netReturn();
		if(!net_return)
		{
			std::cout << "\t\t\t" + name + "netReturn() returned null!\n";
		}
		
		if(initial == 0)
		{
			//std::cout << "PaperAccount percentReturn(): Initial balance is zero!" << std::endl;
			return 0.0L;
		}
		return net_return / initial;
	}
	
	double PaperAccount::elapsedHours() const
	{
		std::string name = "PaperAccount::elapsedHours(): ";
		if(prices.empty())
		{
			std::cout << name + "Price is not set!\n";
			return 0.0;
		}
		double hours = prices.size() * ((double)interval / 3600.0);
		return hours;
	}

	double PaperAccount::netYearReturn() const
	{
		if(!interval)
		{
			return 0.0;
		}
		double timeframe = (double)YEAR / (double)(prices.size() * interval);
		double percent_return = percentReturn();
		double eq = equity();
		if(!eq || !percent_return)
		{
			return 0.0;
		}
		//double year_balance =  compound(equity(), percentReturn(), timeframe);
		double year_balance =  equity() * pow(percentReturn() + 1, timeframe);
		return year_balance - initial;
	}
	
	double PaperAccount::percentYearReturn() const
	{
		if(initial == 0)
		{
			return 0.0;
		}
		return netYearReturn() / initial;
	}
	

	std::vector<double> PaperAccount::winRate() const
	{	
		if((buys + sells) == 0)
		{
			std::cout << "PaperAccount::winRate(): No trades have occurred!" << std::endl;
			return { 0.0, 0.0, 0.0 };
		}
		
		std::vector<double> rates(3);
		
		double lastBuy = 0.0;
		double lastSell = 0.0;
		
		double buywins = 0.0, sellwins = 0.0;
		
		for(unsigned int i = 0; i < actions.size(); i++)
		{
			double buyprice = prices[i] * (1.0 + fee);
			double sellprice = prices[i] * (1.0 - fee);
			//winning buy
			if (actions[i] == ACTION_BUY && buyprice < lastSell)
			{
				buywins++;
				lastBuy = buyprice;
			}
			if (actions[i] == ACTION_SELL && sellprice > lastBuy)
			{
				sellwins++;
				lastSell = sellprice;
			}
		}
		
		rates[0] = (buywins + sellwins) / (double)(buys + sells);
		rates[1] = buywins / (double)buys;
		rates[2] = sellwins / (double)sells;

		return rates;
	}
	
	double PaperAccount::getPrice() const
	{
		std::string name = "PaperAccount::getPrice(): ";
		if(prices.empty())
		{
			std::cout << name + "Price is not set\n";
			return 0.0;
		}
		return prices.back();
	}

	double PaperAccount::getPrice(unsigned int i) const
	{
		std::string name = "PaperAccount::getPrice(i): ";
		if(i >= prices.size())
		{
			std::cout << name + "Index out of range" << std::endl;
			return 0.0L;
		}
		return prices[i];
	}
	
	void PaperAccount::setPrice(double price)
	{
		prices.push_back(price);
		actions.push_back(0);
	}
	
	std::ostream& operator<<(std::ostream& out, const PaperAccount& acc)
	{
		std::vector<double> rates = acc.winRate();
		double eq = acc.equity();
		double percent_return = acc.percentReturn() * 100.0;
		double win_rate = rates[0] * 100.0;
		double buy_win_rate = rates[1] * 100.0;
		double sell_win_rate = rates[2] * 100.0;
		double net_return = acc.netReturn();
		
		double net_year_return = acc.netYearReturn();
		double percent_year_return = acc.percentYearReturn() * 100;
		double elapsed_hours = acc.elapsedHours();
		
		out << "PaperAccount:\n{"
		<< "\n\tBuys\t\t:\t"		<< acc.getBuys()
		<< "\n\tSells\t\t:\t"		<< acc.getSells()
		<< "\n\tInterval\t:\t"		<< acc.getInterval()
		<< "\n\tWindow\t\t:\t"		<< acc.getWindow()
		<< "\n\tElapsed Hrs\t:\t"	<< elapsed_hours
		<< "\n"
		<< "\n\tInitial\t\t:\t"		<< acc.getInitial()
		<< "\n\tShares\t\t:\t"		<< acc.getShares()
		<< "\n\tBalance\t\t:\t"		<< acc.getBalance() 
		<< "\n\tEquity\t\t:\t"		<< eq
		<< "\n"
		<< "\n\tNet Return\t:\t"	<< net_return
		<< "\n\t% Return\t:\t"		<< percent_return << "%"
		<< "\n"
		<< "\n\tYr Return\t:\t"		<< net_year_return
		<< "\n\t% Yr Return\t:\t"	<< percent_year_return << "%"
		<< "\n"
		<< "\n\tWin Rate\t:\t"		<< win_rate << "%"
		<< "\n\tB Win Rate\t:\t"	<< buy_win_rate << "%"
		<< "\n\tS Win Rate\t:\t"	<< sell_win_rate << "%"
		<< "\n}";
		return out;
	}
}
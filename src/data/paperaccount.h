#pragma once

#include <iostream>
#include <vector>
#include <string>

namespace daytrender
{	
	class PaperAccount
	{
	protected:
		double balance = 0.0, shares = 0.0, fee = 0.0, minimum = 0.0, initial = 0.0;
		unsigned int buys = 0, sells = 0, interval = 0, window = 0;
		
		std::vector<double> prices = { 1.0 };
		std::vector<unsigned int> actions = { 0 };
		
	public:
		PaperAccount() = default;
		PaperAccount(double initial, double fee, double minimum,
			unsigned int interval, unsigned int window);
		PaperAccount(const PaperAccount& other);
		PaperAccount(const PaperAccount& other, unsigned int interval, unsigned int window);
			
		void buy(const double& shares);
		void sell(const double& shares);
		
		inline double getInitial() const { return initial; }
		inline double getBalance() const { return balance; }	
		inline double getShares() const { return shares; }
		inline double getFee() const { return fee; }
		inline double getMinimum() const { return minimum; }
		
		double getPrice() const;
		double getPrice(unsigned int i) const;
		void setPrice(double price);
		
		inline unsigned int getBuys() const { return buys; }
		inline unsigned int getSells() const { return sells; }
		inline unsigned int getTrades() const { return buys + sells; }
		inline unsigned int getInterval() const { return interval; }
		inline unsigned int getWindow() const { return window; }
		
		inline void setConstraints(unsigned int interval, unsigned int window)
		{
			this->interval = interval;
			this->window = window;
		}
		
		//non-trivial getters
	
		//the equity at the most recent price
		double equity() const;
		//the net dollar return
		double netReturn() const;
		//the overall percent return
		double percentReturn() const;
		//the amount of elapsed hours during calculation
		double elapsedHours() const;
		//theoretical net return over a year of the same performance
		double netYearReturn() const;
		//theoretical percnet return over a year of the same performance
		double percentYearReturn() const;
		//the ratio of winning trades to losing trades, returns
		//a vector with [0] being the overall, [1] being the buy winrate,
		//and [2] being the sell win rate
		std::vector<double> winRate() const;

		//todo implement buy win rate and sell winrate
		
		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}
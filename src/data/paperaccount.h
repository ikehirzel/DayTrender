#pragma once

#include <iostream>
#include <vector>
#include <string>

namespace daytrender
{	
	class PaperAccount
	{
	private:
		double
			balance = 0.0,
			shares = 0.0,
			fee = 0.0,
			minimum = 0.0,
			initial = 1.0,
			price = 0.0,
			lastActPrice = 0.0;
		
		unsigned int
			buys = 0,
			sells = 0,
			buywins = 0,
			buylosses = 0,
			sellwins = 0,
			selllosses = 0,
			interval = 0,
			window = 0,
			updates = 0;
		
	public:
		PaperAccount() = default;
		PaperAccount(double initial, double fee, double minimum, unsigned int interval,
			unsigned int window);
			
		void buy(double shares);
		void sell(double shares);
		
		inline double getInitial() const { return initial; }
		inline double getBalance() const { return balance; }	
		inline double getShares() const { return shares; }
		inline double getFee() const { return fee; }
		inline double getMinimum() const { return minimum; }
		
		inline double getPrice() const { return price; }
		inline void setPrice(double _price) { updates++; price = _price; };
		
		inline unsigned int getBuys() const { return buys; }
		inline unsigned int getSells() const { return sells; }
		inline unsigned int getTrades() const { return buys + sells; }
		inline unsigned int getInterval() const { return interval; }
		inline unsigned int getWindow() const { return window; }
		
		//non-trivial getters

		double equity() const;
		double netReturn() const;
		double percentReturn() const;
		double elapsedHours() const;
		double avgHourNetReturn() const;
		double avgHourPercentReturn() const;
		double buyWinRate() const;
		double sellWinRate() const;
		double winRate() const;

		//todo implement buy win rate and sell winrate
		
		std::string to_string() const;

		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}
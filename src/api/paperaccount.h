#pragma once

#include <iostream>
#include <vector>
#include <string>

namespace daytrender
{	
	class PaperAccount
	{
	private:
		double _principal = 0.0;
		double _balance = 0.0;
		double _fee = 0.0;
		double _minimum = 0.0;
		double _price = 0.0;
		double _last_act_price = 0.0;
		double _shares = 0.0;
		
		int _buys = 0;
		int _buy_wins = 0;
		int _buy_losses = 0;

		int _sales = 0;
		int _sale_wins = 0;
		int _sale_losses = 0;

		int _interval = 0;
		int _updates = 0;

		std::vector<int> _ranges;
		
	public:
		PaperAccount() = default;
		PaperAccount(double principal, double fee, double minimum, double initial_price,
			int interval, const std::vector<int>& ranges);
			
		void buy(double shares);
		void sell(double shares);
		
		inline double principal() const { return _principal; }
		inline double balance() const { return _balance; }	
		inline double shares() const { return _shares; }
		inline double fee() const { return _fee; }
		inline double minimum() const { return _minimum; }
		
		inline double price() const { return _price; }
		inline void update_price(double price)
		{
			_price = price;
			_updates++; 
		};
		
		inline int buys() const { return _buys; }
		inline int sales() const { return _sales; }
		inline int trades() const { return _buys + _sales; }
		inline int interval() const { return _interval; }
		inline const std::vector<int>& ranges() const { return _ranges; };
		
		//non-trivial getters

		double equity() const;
		double net_return() const;
		double pct_return() const;

		double elapsed_hours() const;

		double avg_net_per_hour() const;
		double avg_pct_per_hour() const;

		double buy_win_rate() const;
		double buy_loss_rate() const;

		double sale_win_rate() const;
		double sale_loss_rate() const;

		double win_rate() const;
		double loss_rate() const;
		
		std::string to_string() const;

		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}
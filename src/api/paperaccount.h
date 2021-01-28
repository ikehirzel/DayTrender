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
		double _minimum = 1.0;
		double _price = 0.0;
		double _last_act_price = 0.0;
		double _shares = 0.0;
		double _leverage = 1.0;
		double _margin_used = 0.0;

		int _buys = 0;
		int _buy_wins = 0;
		int _buy_losses = 0;

		int _sales = 0;
		int _sale_wins = 0;
		int _sale_losses = 0;

		int _interval = 0;
		int _updates = 0;

		std::vector<int> _ranges;
		std::string _error;
		
	public:
		PaperAccount() = default;
		PaperAccount(double principal, int leverage, double fee, double minimum,
			double initial_price, int interval, const std::vector<int>& ranges);
			
		bool buy(double shares);
		bool sell(double shares);
		
		inline double principal() const { return _principal; }
		inline double balance() const { return _balance; }
		inline double shares() const { return _shares; }
		inline double fee() const { return _fee; }
		inline double minimum() const { return _minimum; }
		inline double leverage() const { return _leverage; }
		
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
		double equity() const { return _balance + (_shares * _price) - _margin_used; }
		double buying_power() const
		{
			double bp = _balance * _leverage - _margin_used;
			return (bp >= 0.0 ? bp : 0.0);
		}

		double net_return() const;
		double pct_return() const;

		double net_per_year() const;
		double pct_per_year() const;

		double buy_win_rate() const;
		double buy_loss_rate() const;

		double sale_win_rate() const;
		double sale_loss_rate() const;

		double win_rate() const;
		double loss_rate() const;

		double elapsed_hours() const;
		
		std::string to_string() const;
		inline std::string error() const { return _error; }
		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}
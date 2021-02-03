#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "../data/action.h"

namespace daytrender
{	
	class PaperAccount
	{
	private:
		double _principal = 0.0;
		double _balance = 0.0;
		double _fee = 0.0;
		double _order_minimum = 1.0;
		double _price = 0.0;
		double _shares = 0.0;
		double _leverage = 1.0;
		double _margin_used = 0.0;

		bool _shorting_enabled = false;

		int _buys = 0;
		int _sales = 0;
		double _long_profits = 0.0;
		double _long_losses = 0.0;
		int _long_wins = 0;

		int _interval = 0;
		int _updates = 0;

		typedef bool (PaperAccount::*PaperAction)();

		std::vector<int> _ranges;
		std::string _error;
		
		bool enter_long();
		bool exit_long();
		bool enter_short();
		bool exit_short();

	public:
		PaperAccount() = default;
		PaperAccount(double principal, int leverage, double fee, double order_minimum,
			double initial_price, bool shorting_enabled, int interval, const std::vector<int>& ranges);

		inline bool handle_action(int action)
		{
			switch(action)
			{
			case ENTER_LONG:
				return enter_long();
			case EXIT_LONG:
				return exit_long();
			case ENTER_SHORT:
				return enter_short();
			case EXIT_SHORT:
				return exit_short();
			default:
				return true;
			}
		}
		
		inline double principal() const { return _principal; }
		inline double balance() const { return _balance; }
		inline double shares() const { return _shares; }
		inline double fee() const { return _fee; }
		inline double order_minimum() const { return _order_minimum; }
		inline double leverage() const { return _leverage; }
		inline double long_profits() const { return _long_profits; }
		inline double long_losses() const { return _long_losses; }
		inline double long_movement() const { return _long_profits + _long_losses; }
		
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

		double long_win_rate() const;
		double long_profit_rate() const;

		double elapsed_hours() const;
		
		std::string to_string() const;
		inline std::string error() const { return _error; }
		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}
#ifndef DAYTRENDER_PAPERACCOUNT_H
#define DAYTRENDER_PAPERACCOUNT_H

// local includesifndef
#include <api/action.h>

// standard library
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
		double _order_minimum = 1.0;
		double _price = 0.0;
		double _initial_price = 0.0;
		double _shares = 0.0;
		double _leverage = 1.0;
		double _margin_used = 0.0;
		double _price_sum = 0.0;
		std::vector<double> _return_history;

		bool _shorting_enabled = false;

		int _long_entrances = 0;
		int _long_exits = 0;
		double _long_profits = 0.0;
		double _long_losses = 0.0;
		int _long_win_count = 0;
		int _long_loss_count = 0;

		int _short_entrances = 0;
		int _short_exits = 0;
		double _short_profits = 0.0;
		double _short_losses = 0.0;
		int _short_win_count = 0;
		int _short_loss_count = 0;

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

		inline bool close_position()
		{
			if (_shares > 0.0)
			{
				return exit_long();
			}
			else if (_shares < 0.0)
			{
				return exit_short();
			}
			return true;
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

		inline double short_profits() const { return _short_profits; }
		inline double short_losses() const { return _short_losses; }
		inline double short_movement() const { return _short_profits + _short_losses; }
		
		inline double price() const { return _price; }
		inline void update_price(double price)
		{
			_price = price;
			_price_sum += _price;
			_updates++; 
		};
		
		inline int long_entrances() const { return _long_entrances; }
		inline int long_exits() const { return _long_exits; }
		inline int long_trades() const { return _long_entrances + _long_exits; }

		inline int short_entrances() const { return _short_entrances; }
		inline int short_exits() const { return _short_exits; }
		inline int short_trades() const { return _short_entrances + _short_exits; }

		inline int interval() const { return _interval; }
		inline const std::vector<int>& ranges() const { return _ranges; };
		
		//non-trivial getters
		double equity() const
		{
			return _balance + _shares * _price + (_shares >= 0.0 ? -_margin_used : _margin_used);
		}
		double buying_power() const
		{
			double bp = _balance * _leverage - _margin_used;
			return (bp >= 0.0 ? bp : 0.0);
		}

		double net_return() const;
		double pct_return() const;

		double net_per_year() const;
		double pct_per_year() const;

		double return_volatility() const;
		double sharpe_ratio() const;
		double kelly_criterion() const;

		double long_win_rate() const;
		double long_profit_rate() const;
		double short_win_rate() const;
		double short_profit_rate() const;
		double win_rate() const;
		double loss_rate() const;
		double profit_rate() const;

		double elapsed_hours() const;
		
		std::string to_string() const;
		inline std::string error() const { return _error; }
		friend std::ostream& operator<<(std::ostream& out, const PaperAccount& acc);
	};
}

#endif

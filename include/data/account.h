#pragma once

namespace daytrender
{
	class Account
	{
	private:
		double _balance = 0.0;
		double _buying_power = 0.0;
		double _margin_used = 0.0;
		double _equity = 0.0;
		unsigned _leverage = 0.0;
		bool _shorting_enabled = false;

	public:
		Account() = default;
		Account(double balance, double buying_power, double margin_used, double equity, int leverage, bool shorting_enabled);

		inline double balance() const { return _balance; }
		inline double buying_power() const { return _buying_power; }
		inline double margin_used() const { return _margin_used; }
		inline double equity() const { return _equity; }
		inline unsigned leverage() const { return _leverage; }
		inline bool shorting_enabled() const { return _shorting_enabled; }
	};
}

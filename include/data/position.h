#ifndef DAYTRENDER_POSITION_H
#define DAYTRENDER_POSITION_H

namespace daytrender
{
	class Position
	{
	private:
		double _amt_invested = 0.0;
		double _fee = 0.0;
		double _minimum = 0.0;
		double _price = 0.0;
		double _shares = 0.0;

	public:
		Position() = default;
		Position(double amt_invested, double fee, double minimum,
			double price, double shares);

		inline double amt_invested() const { return _amt_invested; }
		inline double fee() const { return _fee; }
		inline double minimum() const { return _minimum; }
		inline double price() const { return _price; }
		inline double shares() const { return _shares; }
	};
}

#endif

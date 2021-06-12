#ifndef DAYTRENDER_CANDLE_H
#define DAYTRENDER_CANDLE_H

// standard library
#include <iostream>
#include <string>

namespace daytrender
{
	class Candle
	{
	private:
		double _open = 0.0;
		double _high = 0.0;
		double _low = 0.0;
		double _close = 0.0;
		double _volume = 0.0;

	public:
		Candle() = default;
		Candle(double open, double high, double low, double close, double volume);

		inline double o() const { return _open; }
		inline double open() const { return _open; }

		inline double h() const { return _high; }
		inline double high() const { return _high; }

		inline double l() const { return _low; }
		inline double low() const { return _low; }

		inline double c() const { return _close; }
		inline double close() const { return _close; }

		inline double v() const { return _volume; }
		inline double volume() const { return _volume; }

		std::string to_string() const;
		friend std::ostream& operator<<(std::ostream& out, const Candle& candle);
	};
}

#endif

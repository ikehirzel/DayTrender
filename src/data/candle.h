#pragma once

#include <iostream>
#include <vector>

namespace daytrender
{
	struct candle
	{
		unsigned long interval = 0;
		double open = 0.0, high = 0.0, low = 0.0, close = 0.0, volume = 0.0;
		candle() = default;
		candle(double _open, double _high, double _low, double _close, double _volume)
		{
			open = _open;
			high = _high;
			low = _low;
			close = _close;
			volume = _volume;
		}
	};

	struct candleset
	{
		candle* candles = nullptr;
		int size = 0;
		int interval = 0;

		candleset() = default;
		candleset(int _size, int _interval)
		{
			init(_size, _interval);
		}

		void clear()
		{
			if (candles)
			{
				delete[] candles;
				candles = nullptr;
			}
			size = 0;
			interval = 0;
		}

		inline void init(int _size, int _interval)
		{
			clear();
			size = _size;
			interval = _interval;
			candles = new candle[size];
		}

		inline candleset get_slice(int _offset, int _size)
		{
			candleset out;
			out.candles = candles + _offset;
			out.size = _size;
			out.interval = interval;
			return out;
		}

		inline candle& operator[] (int index) { return candles[index]; }
		inline const candle& operator[] (int index) const { return candles[index]; }
		inline const candle& back() const { return candles[size - 1]; }
		inline bool empty() const { return size == 0; }
	};


	struct candleset_data
	{
		candleset candles;
		unsigned interval;
	};

}

#pragma once

#include <string>
#include <iostream>

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

	class CandleSet
	{
	private:
		Candle* _data = nullptr;
		unsigned _size = 0;
		unsigned _interval = 0;
		unsigned _shamt = 0;
		bool _slice = false;
		mutable const char* _error;
	public:
		CandleSet() = default;
		CandleSet(int size, int interval);
		CandleSet(const CandleSet& other);
		CandleSet(const CandleSet& other, unsigned offset, unsigned size, unsigned shamt);
		~CandleSet();

		inline CandleSet slice(unsigned offset, unsigned size, unsigned shamt)
		{
			return CandleSet(*this, offset, size, shamt);
		}

		CandleSet& operator=(const CandleSet& other);

		inline Candle& operator[] (int index)
		{
			// this account for shamt but allows for only one check
			unsigned i = index + (int)_shamt;
			if (i >= _size)
			{
				_error = "out of bounds memory access! returned *data";
				return *_data;
			}
			return _data[i];
		}

		inline const Candle& operator[] (int index) const
		{
			// this account for shamt but allows for only one check
			unsigned i = index + (int)_shamt;
			if (i >= _size)
			{
				_error = "out of bounds memory access! returned *data";
				return *_data;
			}
			return _data[i];
		}

		inline const Candle& back(int index = 0) const
		{
			unsigned i = index + (int)_shamt;
			if (i >= _size)
			{
				_error = "out of bounds memory access! returned *data";
				return *_data;
			}
			return _data[(_size - 1) - i];
		}

		inline const Candle& front(int index = 0) const
		{
			unsigned i = index + (int)_shamt;
			if (i >= _size)
			{
				_error = "out of bounds memory access! returned *data";
				return *_data;
			}
			return _data[i];
		}

		inline bool empty() const { return _size == 0; }
		inline int size() const { return _size; }
		inline int interval() const { return _interval; }
		inline Candle* data() const { return _data; }
		const char* error() { return _error; _error = nullptr; }
	};
}

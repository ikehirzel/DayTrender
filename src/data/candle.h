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
		unsigned _end = 0;
		bool _slice = false;
		mutable const char* _error = nullptr;

		CandleSet(Candle* parent_data, unsigned parent_size, int parent_interval, unsigned offset, unsigned size, unsigned end);

	public:
		CandleSet() = default;
		CandleSet(unsigned size, unsigned end, int interval);
		CandleSet(CandleSet&& other);
		CandleSet(const CandleSet& other);
		~CandleSet();

		CandleSet& operator=(const CandleSet& other);
		inline CandleSet slice(unsigned offset, unsigned size, unsigned end)
		{
			return CandleSet(_data, _size, _interval, offset, size, end);
		}

		inline Candle& set(unsigned index)
		{
			// this account for shamt but allows for only one check
			if (index >= _size)
			{
				_error = "out of bounds memory access";
				return *_data;
			}
			return _data[index];
		}

		inline const Candle& operator[] (int index) const
		{
			// this account for shamt but allows for only one check
			unsigned i = index + (int)(_size - _end);
			//std::cout << "getting candle : " << i << std::endl;
			if (i >= _size)
			{
				_error = "out of bounds memory access";
				return *_data;
			}
			return _data[i];
		}

		inline const Candle& back(unsigned index = 0) const
		{
			if (index >= _size)
			{
				_error = "out of bounds memory access";
				return *_data;
			}
			return _data[(_size - 1) - index];
		}

		inline const Candle& front(unsigned index = 0) const
		{
			if (index >= _size)
			{
				_error = "out of bounds memory access";
				return *_data;
			}
			return _data[index];
		}

		inline bool is_slice() const { return _slice; }
		inline bool empty() const { return _size == 0; }
		inline unsigned size() const { return _size; }
		inline unsigned end() const { return _end; }
		inline int interval() const { return _interval; }
		const char* error() const { return _error; }
	};
}

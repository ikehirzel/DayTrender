#ifndef DAYTRENDER_CANDLE_H
#define DAYTRENDER_CANDLE_H

// daytrender includes
#include "result.h"

// standard library
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
		bool _slice = false;

		// constructor for making slices
		CandleSet(Candle* parent_data, unsigned parent_size,
			unsigned parent_interval, unsigned offset, unsigned size);

	public:
		CandleSet() = default;
		CandleSet(unsigned size, unsigned interval);
		CandleSet(CandleSet&& other);
		CandleSet(const CandleSet& other);
		~CandleSet();

		CandleSet& operator=(const CandleSet& other);
		inline Result<CandleSet> slice(unsigned offset, unsigned size)
		{
			if (!_data) return "parent data is uninitialized";
			if (offset + size > _size) return "slice range is out of parent's bounds";
			return CandleSet(_data, _size, _interval, offset, size);
		}

		inline Candle& set(unsigned index)
		{
			// this account for shamt but allows for only one check
			if (index >= _size)
			{
				return *_data;
			}
			return _data[index];
		}

		inline const Candle& operator[] (int index) const
		{
			// this account for shamt but allows for only one check
			unsigned i = index;
			//std::cout << "getting candle : " << i << std::endl;
			if (i >= _size)
			{
				return *_data;
			}
			return _data[i];
		}

		inline const Candle& back(unsigned index = 0) const
		{
			if (index >= _size)
			{
				return *_data;
			}
			return _data[(_size - 1) - index];
		}

		inline const Candle& front(unsigned index = 0) const
		{
			if (index >= _size)
			{
				return *_data;
			}
			return _data[index];
		}

		inline bool is_slice() const { return _slice; }
		inline bool empty() const { return _size == 0; }
		inline unsigned size() const { return _size; }
		inline int interval() const { return _interval; }
	};
}

#endif

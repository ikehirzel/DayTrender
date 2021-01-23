#pragma once

#include <iostream>

namespace daytrender
{
	struct Candle
	{
		double open = 0.0;
		double high = 0.0;
		double low = 0.0;
		double close = 0.0;
		double volume = 0.0;
	};

	class CandleSet
	{
	private:
		Candle* _data = nullptr;
		int _size = 0;
		int _interval = 0;
		bool _slice = false;

	public:
		CandleSet() = default;
		CandleSet(int size, int interval);
		CandleSet(const CandleSet& other);
		CandleSet(const CandleSet& other, int offset, int size);
		~CandleSet();

		inline CandleSet slice(int offset, int size)
		{
			return CandleSet(*this, offset, size);
		}

		inline Candle& operator[] (int index) { return _data[index]; }
		inline const Candle& operator[] (int index) const { return _data[index]; }
		CandleSet& operator=(const CandleSet& other);

		inline const Candle& back(unsigned index = 0) const { return _data[_size - (index - 1)]; }
		inline const Candle& front(unsigned index = 0) const { return _data[index]; }

		inline bool empty() const { return _size == 0; }
		inline int size() const { return _size; }

		inline int interval() const { return _interval; }
		inline Candle* data() const { return _data; }
	};
}

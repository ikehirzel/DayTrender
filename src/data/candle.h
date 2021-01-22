#pragma once

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
		Candle* candles_ = nullptr;
		int size_ = 0;
		int interval_ = 0;
		bool clone_ = false;

	public:
		CandleSet() = default;
		CandleSet(int size, int interval)
		{
			size_ = size;
			interval_ = interval;
			candles_ = new Candle[size];
		}

		CandleSet(CandleSet* other, int offset, int size)
		{
			clone_ = true;
			size_ = size;
			if (offset + size_ > other->size()) size_ = 0;
			interval_ = other->interval();
			candles_ = other->data() + offset;
		}

		~CandleSet()
		{
			if (!clone_) delete[] candles_;
		}

		inline CandleSet get_slice(int offset, int size)
		{
			return CandleSet(this, offset, size);
		}

		inline Candle& operator[] (int index) { return candles_[index]; }
		inline const Candle& operator[] (int index) const { return candles_[index]; }
		inline const Candle& back() const { return candles_[size_ - 1]; }
		inline bool empty() const { return size_ == 0; }
		inline int size() const { return size_; }
		inline int interval() const { return interval_; }
		inline Candle* data() const { return candles_; }
	};
}

#pragma once

#include "actionenum.h"
#include "candle.h"

#include <stdexcept>
#include <vector>

namespace daytrender
{
	class Indicator
	{
	private:
		double* data_ = nullptr;
		int size_ = 0;

	public:
		const char* type = nullptr;
		const char* label = nullptr;

		Indicator() = default;
		Indicator(int size)
		{
			size_ = size;
			data_ = new double[size_];
		}

		~Indicator()
		{
			delete[] data_;
		}

		double& operator[](unsigned pos)
		{
			if (pos >= size_)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
		
			return data_[pos];
		}

		double back(unsigned pos = 0) const
		{
			if (pos >= size_)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
			return data_[(size_ - 1) - pos];
		}

		double front(unsigned pos = 0) const
		{
			if (pos >= size_)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
			return data_[pos];
		}

		inline int size() const { return size_; }
	};
	
	class AlgorithmData
	{
	private:
		Indicator* dataset_ = nullptr;
		int action_ = 0;
		const char* label_ = nullptr;
		const char* error_ = nullptr;

	public:
		std::vector<int> ranges;
		CandleSet candles;
		
		inline void do_nothing() { action_ = Action::NOTHING; }
		inline void sell() { action_ = Action::SELL; }
		inline void buy() { action_ = Action::BUY; }

		AlgorithmData() = default;
		AlgorithmData(const std::vector<int>& ranges)
		{
			this->ranges = ranges;
			dataset_ = new Indicator[ranges.size() - 1];
			for (int i = 1; i < ranges.size(); i++)
			{
				dataset_[i - 1] = Indicator(ranges[i]);
			}
		}
		
		~AlgorithmData()
		{
			delete[] dataset_;
		}

		Indicator& operator[](unsigned index)
		{
			return dataset_[index];
		}

		inline int action() const { return action_; }
		inline int size() const { return ranges.size() - 1; }
		inline void set_label(const char* label) { label_ = label; }
		inline void flag_error(const char* error) { error_ = error; }

		inline const char* label() const { return label_; }
		inline const char* error() const { return error_; }

	};
}

#pragma once

#include "actionenum.h"

#include <string>
#include <vector>
#include <unordered_map>
#include "candle.h"

namespace daytrender
{
	struct indicator
	{
		const char* type = nullptr;
		const char* label = nullptr;
		double* data;
		int size;

		void create(int _size)
		{
			size = _size;
			data = new double[size];
		}

		void clear()
		{
			size = 0;
			delete[] data;
			data = nullptr;
		}

		double& operator[](unsigned pos)
		{
			if (pos >= size)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
		
			return data[pos];
		}

		double back(unsigned pos = 0) const
		{
			if (pos >= size)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
			return data[(size - 1) - pos];
		}

		double front(unsigned pos = 0) const
		{
			if (pos >= size)
			{
				throw std::out_of_range("attempt to access indicator element outside of range");
			}
			return data[pos];
		}
	};
	
	struct algorithm_data
	{
		const char* type = nullptr;
		const char* label = nullptr;
		const char* err = nullptr;
		const int *ranges = nullptr;
		candleset candles;
		indicator* dataset = nullptr;
		int ranges_size = 0;
		int action = 0;
		
		inline void do_nothing() { action = Action::NOTHING; }
		inline void sell() { action = Action::SELL; }
		inline void buy() { action = Action::BUY; }

		inline void create(const int* _ranges, int _ranges_size)
		{
			ranges_size = _ranges_size;
			ranges = _ranges;

			dataset = new indicator[ranges_size - 1];
			for (int i = 0; i < ranges_size - 1; i++)
			{
				dataset[i].create(ranges[0]);
			}
		}

		inline void clear()
		{
			for (int i = 0; i < ranges_size - 1; i++)
			{
				dataset[i].clear();
			}
			delete[] dataset;
		}
	};
}

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../data/candle.h"

#if defined(_WIN32) || defined(_WIN64)
#define ALGORITHM_EXTENSION ".dll"
#elif defined(linux) || defined(__unix__)
#define ALGORITHM_EXTENSION ".so"
#endif


namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	typedef std::pair<std::string, std::vector<double>> indicator_data;
	typedef std::unordered_map<std::string, indicator_data> indicator_dataset;
	typedef std::pair<indicator_dataset, unsigned int> algorithm_data;

	class TradeAlgorithm
	{
	protected:
		bool bound = false;
		hirzel::Plugin* handle = nullptr;
		std::string name, filename;
		
	public:
		TradeAlgorithm(const std::string& filename);
		~TradeAlgorithm();

		algorithm_data process(const candleset& candles, unsigned int index,
			unsigned int window);
		inline std::string getName() const { return name; }
		inline bool isBound()
		{
			return bound;
		}
	};
}

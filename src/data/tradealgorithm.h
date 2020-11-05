#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "candle.h"

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
	struct indicator_data
	{
		std::string label;
		std::vector<double> data;
	};
	typedef std::unordered_map<std::string, indicator_data> indicator_dataset;
	struct algorithm_data
	{
		std::unordered_map<std::string, indicator_data> dataset;
		unsigned int action;
	};

	class TradeAlgorithm
	{
	protected:
		bool bound = false;
		hirzel::Plugin* handle = nullptr;
		std::string name, filename;
		
	public:
		TradeAlgorithm(const std::string& filename);
		~TradeAlgorithm();

		algorithm_data process(const candleset& candles);
		inline std::string getName() const { return name; }
		inline bool isBound()
		{
			return bound;
		}
	};
}

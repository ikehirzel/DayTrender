#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "candle.h"
#include "algodefs.h"

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

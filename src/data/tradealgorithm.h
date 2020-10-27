#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "../data/candle.h"

namespace tinyplug
{
	class Plugin;
}

namespace daytrender
{
	typedef std::pair<std::string, std::vector<double>> indicator_data;
	typedef std::map<std::string, indicator_data> indicator_dataset;
	typedef std::pair<indicator_dataset, unsigned int> algorithm_data;

	class TradeAlgorithm
	{
	protected:
		std::string name, filename;
		tinyplug::Plugin* handle;
		
	public:
		TradeAlgorithm(const std::string& filename);
		~TradeAlgorithm();

		algorithm_data process(const candleset& candles, unsigned int index,
			unsigned int window);
		inline std::string getName() const { return name; }
	};
}

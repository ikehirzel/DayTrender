#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../data/algotypes.h"

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	typedef void(*algorithm_func)(algorithm_data&);

	class TradeAlgorithm
	{
	private:
		bool bound = false;
		hirzel::Plugin* handle = nullptr;
		std::string filename;
		int ranges_count = 0;
		// functions from 
		algorithm_func algo;

	public:
		TradeAlgorithm(const std::string& _filepath);
		~TradeAlgorithm();

		algorithm_data process(const candleset& candles, const std::vector<int>& ranges);
		inline const std::string& get_filename() const { return filename; };
		inline int get_ranges_count() const { return ranges_count; }
		inline bool is_bound() const { return bound; }
	};
}

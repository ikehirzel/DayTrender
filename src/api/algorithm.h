#pragma once

#include <string>
#include <vector>

#include "../data/algotypes.h"

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	typedef void(*AlgorithmFunc)(AlgorithmData&);

	class Algorithm
	{
	private:
		bool bound = false;
		hirzel::Plugin* handle = nullptr;
		std::string filename;
		int ranges_count = 0;
		// functions from 
		AlgorithmFunc algo;

	public:
		Algorithm(const std::string& _filepath);
		~Algorithm();

		AlgorithmData process(const CandleSet& candles, const std::vector<int>& ranges) const;
		inline const std::string& get_filename() const { return filename; };
		inline int get_ranges_count() const { return ranges_count; }
		inline bool is_bound() const { return bound; }
	};
}

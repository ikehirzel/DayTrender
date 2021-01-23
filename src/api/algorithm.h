#pragma once

#include <string>
#include <vector>

#include "../data/algorithmdata.h"

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	class Algorithm
	{
	private:
		bool _bound = false;
		hirzel::Plugin* _plugin = nullptr;
		std::string _filename;
		int _ranges_count = 0;
		void(*_algorithm_ptr)(AlgorithmData&);

	public:
		Algorithm(const std::string& _filepath);
		~Algorithm();

		AlgorithmData process(const CandleSet& candles, const std::vector<int>& ranges) const;
		inline const std::string& get_filename() const { return _filename; };
		inline int get_ranges_count() const { return _ranges_count; }
		inline bool is_bound() const { return _bound; }
	};
}

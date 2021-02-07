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
		hirzel::Plugin* _plugin = nullptr;
		int _indicator_count = 0;
		int _data_length = 0;
		void(*_algorithm_ptr)(AlgorithmData&) = nullptr;
		std::string _filename;

	public:
		Algorithm(const std::string& _filepath);
		~Algorithm();

		AlgorithmData process(const CandleSet& candles, const std::vector<int>& ranges) const;
		inline const std::string& filename() const { return _filename; };
		inline int indicator_count() const { return _indicator_count; }
		inline bool is_bound() const { return _algorithm_ptr != nullptr; }
		inline int data_length() const { return _data_length; }
	};
}

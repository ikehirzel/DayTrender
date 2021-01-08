#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "algotypes.h"

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
		std::string filename;
		int args = 0;
		
	public:
		TradeAlgorithm(const std::string& _filepath);
		~TradeAlgorithm();

		bool process(algorithm_data& data);
		inline const std::string& get_filename() const { return filename; };
		inline int arg_count() const { return args; }
		inline bool is_bound() const { return bound; }
	};
}

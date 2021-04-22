#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../data/strategydata.h"

namespace hirzel
{
	class Plugin;
}

namespace daytrender
{
	class Strategy
	{
	private:
		static std::unordered_map<std::string, hirzel::Plugin*> _plugins;

		// plugin info
		bool _bound = false;
		std::string _filename;
		Plugin* _plugin = nullptr;
		//
		int _indicator_count = 0;
		int _data_length = 0;
		void(*_execute)(StrategyData&) = nullptr;

	public:
		Strategy() = default;
		Strategy(const std::string& _filepath);

		static void free_plugins();

		StrategyData execute(const CandleSet& candles, const std::vector<int>& ranges) const;
		inline const std::string& filename() const { return _filename; };
		inline int indicator_count() const { return _indicator_count; }
		inline bool is_bound() const { return _execute != nullptr; }
		inline int data_length() const { return _data_length; }
	};
}

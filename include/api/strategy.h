#ifndef DAYTRENDER_STRATEGY_H
#define DAYTRENDER_STRATEGY_H

// local includes
#include <data/chart.h>
#include <data/result.h>

// standard library
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// external libraries
#include <hirzel/plugin.h>

namespace daytrender
{
	class Strategy
	{
	private:
		static std::unordered_map<std::string, std::shared_ptr<hirzel::Plugin>> _plugins;

		// plugin info
		std::string _filename;
		std::shared_ptr<hirzel::Plugin> _plugin = nullptr;
		//
		int _indicator_count = 0;
		int _data_length = 0;
		const char *(*_execute)(Chart*) = nullptr;

	public:
		Strategy() = default;
		Strategy(const std::string& filename, const std::string& dir);

		Result<Chart> execute(const PriceHistory& candles,
			const std::vector<int>& ranges) const;
		inline const std::string& filename() const { return _filename; };
		inline int indicator_count() const { return _indicator_count; }
		inline bool is_bound() const { return (bool)_plugin; }
		inline int data_length() const { return _data_length; }
	};
}

#endif

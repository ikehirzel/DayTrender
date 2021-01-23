#include "algorithm.h"
#include <hirzel/plugin.h>
#include <hirzel/fountain.h>
#include <hirzel/strutil.h>

namespace daytrender
{
	Algorithm::Algorithm(const std::string& _filepath)
	{
		_filename = hirzel::str::get_filename(_filepath);
		_plugin = new hirzel::Plugin(_filepath, { "ranges_size", "algorithm" });
		
		if (!_plugin->all_bound())
		{
			errorf(_plugin->get_error());
			return;
		}

		_ranges_count = _plugin->execute<int>("ranges_size");
		_algorithm_ptr = (void(*)(AlgorithmData&))_plugin->get_func("algorithm");
		_bound = true;
		successf("Successfully loaded algorithm: '%s'", _filename);
	}

	Algorithm::~Algorithm()
	{
		delete _plugin;
	}
	
	AlgorithmData Algorithm::process(const CandleSet& candles, const std::vector<int>& ranges) const
	{
		AlgorithmData data(ranges, candles);

		if (!_algorithm_ptr)
		{
			errorf("%s: algorithm function is now bound!", _filename);
			return data;
		}

		_algorithm_ptr(data);

		if (data.error())
		{
			std::string args_glob;
			const std::vector<int>& ranges = data.ranges();
			for (int i = 0; i < ranges.size(); i++)
			{
				if (i > 0) args_glob += ", ";
				args_glob += std::to_string(ranges[i]);
			}
			errorf("%s(%s): %s", data.label(), args_glob, data.error());
		}

		return data;
	}
}
